/*
  ==============================================================================

    CL1BCompressor.h
    Ported from JClones_CL1B.jsfx

    Original Author: JClones
    Original Source: Docs/Plugins_source_code/JClones_CL1B.jsfx
    License: MIT
    GitHub: https://github.com/JClones/JSFXClones

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Optical compressor with smooth, warm, musical compression.
    Fixed parameters for AnalogChannel: Ratio 6:1, Fixed A/R mode, Output 0dB

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

class CL1BCompressor
{
public:
    CL1BCompressor()
    {
        currentSampleRate = 44100.0;
        initTables();
        reset();
    }

    void reset()
    {
        lpf1_state = 0.0f;
        lpf2_state = 0.0f;
        level_state = 0.0f;
        post_eq_s1 = 0.0f;
        post_eq_s2 = 0.0f;
    }

    void setSampleRate(double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Time constants from original (lines 170-176)
        float lpf1_attack_sec = 1.324200f * 0.001f;
        float lpf1_release_sec = 1.782562f * 0.001f;
        float lpf2_attack_sec = 28.011420f * 0.001f;
        float lpf2_release_sec = 26.260180f * 0.001f;
        float release_sec = 5.898f;

        // Calculate filter coefficients (lines 178-185)
        lpf1_attack = std::exp(-1.0f / (float)(sampleRate * lpf1_attack_sec));
        lpf1_release = std::exp(-1.0f / (float)(sampleRate * lpf1_release_sec));
        lpf2_attack = std::exp(-1.0f / (float)(sampleRate * lpf2_attack_sec));
        lpf2_release = std::exp(-1.0f / (float)(sampleRate * lpf2_release_sec));

        release_k = std::exp(-1.0f / (float)(sampleRate * release_sec));

        post_eq_k = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * (20000.0f / (float)sampleRate));

        initTables();
    }

    void setParameters(float thresholdDB)
    {
        // Fixed parameters for AnalogChannel
        float ratio = 4.0f;  // slider1 (changed from 6.0f to 4.0f)
        float attack_release_mode = 0.0f;  // slider5 = 0 (Fixed mode)
        float output_gain_dB = 0.0f;  // slider6

        // Normalize ratio from [2, 10] to [0, 1] for table lookup (line 671)
        float ratio_normalized = (ratio - 2.0f) / 8.0f;

        // Calculate T3 and T10 from ratio (lines 201-202)
        T3 = interpolate_exp(ratio_normalized, table3_exp, 25, false);
        T10 = interpolate_exp(ratio_normalized, table10_exp, 25, false);

        // Calculate T4 from T3 (line 206)
        T4 = interpolate_lin(T3, table4_lin, 24);

        // Calculate T7 from threshold (line 210)
        T7 = DB_TO_K(-40.0f - thresholdDB);

        // Calculate T11 from output gain (line 211)
        T11 = DB_TO_K(-30.0f + output_gain_dB);

        // attack_release_mode (Fixed = 0)
        this->attack_release_mode = (int)attack_release_mode;
    }

    float process(float input)
    {
        // Process mono (using x1/y1 path from stereo algorithm)

        // Feedback signal path (lines 222-224)
        float inv_gr = lpf1_state * 0.2998201f + lpf2_state * 0.079904087f;
        float gain_reduction = 0.0029900903f / clamp(inv_gr + 0.0029900903f);

        // Feedback sidechain path (lines 228-239)
        float T5 = interpolate_exp(inv_gr, table5_exp, 25, false);
        float T6 = interpolate_lin(T5, table6_lin, 24);

        float A1 = 0.01193628f;
        float B1 = 0.9323384f;
        float A2 = 0.4595526f;
        float B2 = 1.0f;

        float m1 = (A1 * T6 + B1 * (1.0f - T6));
        float m2 = (A2 * T6 + B2 * (1.0f - T6));

        float mult = m1 * (1.0f - T4) + m2 * T4;

        float T2_on = 0.08098298f;

        // Detector path (lines 244-250)
        float input1 = (input * T2_on) * mult * T7;

        float T12_1 = interpolate_exp(input1, table12_exp_neg, 48, true);

        float level_1 = std::abs(T12_1);  // mono, no stereo-link needed

        // Attack/Release (lines 252-271)
        float combined_level;

        if (attack_release_mode == AR_FIXED)
        {
            // Fixed mode (line 264)
            combined_level = level_1;
        }
        else
        {
            // This path won't be used in AnalogChannel (always Fixed mode)
            // But included for completeness
            if (level_1 >= level_state)
            {
                // attack
                level_state = std::min(level_state + T8, level_1);
            }
            else
            {
                // release
                level_state = std::max(level_state * release_k - T9, level_1);
            }

            if (attack_release_mode == AR_MANUAL)
            {
                combined_level = level_state;
            }
            else  // AR_FIXED_MANUAL
            {
                combined_level = std::max(level_1, level_state);
            }
        }

        // Table 13 lookup (line 273)
        float T13 = interpolate_lin(combined_level, table13_lin, 252);

        // LPF1 (lines 276-278)
        float lpf1_k = (T13 > lpf1_state) ? lpf1_attack : lpf1_release;
        lpf1_state = T13 + (lpf1_state - T13) * lpf1_k;

        // LPF2 (lines 280-283)
        float lpf2_k = (T13 > lpf2_state) ? lpf2_attack : lpf2_release;
        lpf2_state = T13 + (lpf2_state - T13) * lpf2_k;

        // Output (line 286)
        float y1 = input * T10 * T11 * gain_reduction * 33.768673f;

        // Post-EQ high shelf at 20kHz (line 290)
        post_eq_s1 += (y1 - post_eq_s1) * post_eq_k;

        return post_eq_s1;
    }

    float getGainReductionDB() const
    {
        float inv_gr = lpf1_state * 0.2998201f + lpf2_state * 0.079904087f;
        float gain_reduction = 0.0029900903f / clamp(inv_gr + 0.0029900903f);
        return juce::Decibels::gainToDecibels(gain_reduction);
    }

private:
    // Constants (lines 164-166)
    enum AttackReleaseMode
    {
        AR_FIXED = 0,
        AR_FIXED_MANUAL = 1,
        AR_MANUAL = 2
    };

    // Lookup tables (lines 299-650)
    float table3_exp[25];
    float table4_lin[24];
    float table5_exp[25];
    float table6_lin[24];
    float table8_lin[46];
    float table9_lin[24];
    float table10_exp[25];
    float table12_exp_neg[48];
    float table13_lin[252];

    // State variables
    float lpf1_state, lpf2_state;
    float level_state;
    float post_eq_s1, post_eq_s2;

    // Filter coefficients
    float lpf1_attack, lpf1_release;
    float lpf2_attack, lpf2_release;
    float release_k, post_eq_k;

    // Parameters
    float T3, T4, T7, T10, T11;
    float T8, T9;  // For manual attack/release (not used in Fixed mode)
    int attack_release_mode;

    double currentSampleRate;

    // Helper functions
    static float DB_TO_K(float x)
    {
        return std::pow(10.0f, x / 20.0f);
    }

    static float clamp(float x)
    {
        return std::min(std::max(x, -0.99999988f), 0.99999988f);
    }

    // frexp/ldexp for exponential interpolation (lines 59-85)
    static float frexp_custom(float x, int& exp_out)
    {
        float sign = 1.0f;

        if (x < 0.0f)
        {
            x = -x;
            sign = -1.0f;
        }

        float M_LN2 = std::log(2.0f);
        exp_out = (int)std::ceil(std::log(x) / M_LN2);

        float a1 = x / std::pow(2.0f, (float)exp_out);

        if (a1 == 1.0f)
        {
            a1 = 0.5f;
            exp_out += 1;
        }

        return sign * a1;
    }

    static float ldexp_custom(float a, int b)
    {
        return a * std::pow(2.0f, (float)b);
    }

    // Linear interpolation (lines 92-114)
    float interpolate_lin(float x, float* table_lin, int table_size)
    {
        x = clamp(x);

        if (x < 0.0f)
            x = 0.0f;

        x *= (table_size - 1);

        int index_int = (int)std::floor(x);

        if (index_int == table_size - 1)
        {
            return table_lin[table_size - 1];
        }
        else
        {
            float index_frac = x - index_int;
            return table_lin[index_int] * (1.0f - index_frac) + table_lin[index_int + 1] * index_frac;
        }
    }

    // Exponential interpolation (lines 116-158)
    float interpolate_exp(float x, float* table_exp, int table_size, bool is_neg)
    {
        float* table_ptr = table_exp;

        if (is_neg)
        {
            // table_exp size is [48], with center index 23
            table_ptr = table_exp + 23;
        }

        x = clamp(x);

        // WTF with JSFX: (x == 0.0 doesn't work), so check against very small value
        if (x * 1000000.0f == 0.0f)
        {
            return table_ptr[23];
        }
        else
        {
            int exp;
            float mant = frexp_custom(x, exp);

            int index = 1 - exp;

            if (index < 0)
                index = 0;

            float frac;

            if (index > 22)
            {
                frac = ldexp_custom(mant, 22 + exp);
                index = 23;
            }
            else
            {
                if (mant <= 0.0f)
                {
                    frac = (mant + 0.5f) * 2.0f;
                }
                else
                {
                    frac = (mant - 0.5f) * 2.0f;
                }
            }

            if (x < 0.0f && is_neg)
            {
                index = -index;   // negative index
                frac = frac + 1.0f;  // negative frac
            }

            return frac * (table_ptr[index] - table_ptr[index + 1]) + table_ptr[index + 1];
        }
    }

    // Initialize all lookup tables (lines 299-650)
    void initTables()
    {
        // T3 (lines 302-316)
        table3_exp[0] = 0.999999f;
        table3_exp[1] = 0.99f;
        table3_exp[2] = 0.5626293f;
        table3_exp[3] = 0.2993541f;
        table3_exp[4] = 0.1536661f;
        table3_exp[5] = 0.07558671f;
        table3_exp[6] = 0.036547f;
        table3_exp[7] = 0.01702715f;

        for (int i = 8; i < 25; ++i)
            table3_exp[i] = 0.01f;

        // T4 (lines 319-343)
        table4_lin[0] = 0.0f;
        table4_lin[1] = 0.03416149f;
        table4_lin[2] = 0.07852706f;
        table4_lin[3] = 0.1228926f;
        table4_lin[4] = 0.1672582f;
        table4_lin[5] = 0.2116238f;
        table4_lin[6] = 0.2559893f;
        table4_lin[7] = 0.3003549f;
        table4_lin[8] = 0.3447205f;
        table4_lin[9] = 0.3890861f;
        table4_lin[10] = 0.4334517f;
        table4_lin[11] = 0.4778172f;
        table4_lin[12] = 0.5221828f;
        table4_lin[13] = 0.5665483f;
        table4_lin[14] = 0.6109139f;
        table4_lin[15] = 0.6552795f;
        table4_lin[16] = 0.6996451f;
        table4_lin[17] = 0.7440106f;
        table4_lin[18] = 0.7883762f;
        table4_lin[19] = 0.8327418f;
        table4_lin[20] = 0.8771074f;
        table4_lin[21] = 0.921473f;
        table4_lin[22] = 0.9658385f;
        table4_lin[23] = 0.999999f;

        // T5 (lines 346-371)
        table5_exp[0] = 0.01f;
        table5_exp[1] = 1.0f;
        table5_exp[2] = 0.9947661f;
        table5_exp[3] = 0.9844928f;
        table5_exp[4] = 0.9651101f;
        table5_exp[5] = 0.9302186f;
        table5_exp[6] = 0.8630559f;
        table5_exp[7] = 0.755419f;
        table5_exp[8] = 0.6082814f;
        table5_exp[9] = 0.4397123f;
        table5_exp[10] = 0.2796561f;
        table5_exp[11] = 0.162245f;
        table5_exp[12] = 0.08780019f;
        table5_exp[13] = 0.04508f;
        table5_exp[14] = 0.02209106f;
        table5_exp[15] = 0.01019185f;
        table5_exp[16] = 0.004130001f;
        table5_exp[17] = 0.001069335f;
        table5_exp[18] = 0.00001000000f;
        table5_exp[19] = 0.00001000000f;
        table5_exp[20] = 0.00001000000f;
        table5_exp[21] = 0.00001000000f;
        table5_exp[22] = 0.00001000000f;
        table5_exp[23] = 0.00001000000f;
        table5_exp[24] = 0.00001000000f;

        // T6 (lines 374-398)
        table6_lin[0] = 0.0f;
        table6_lin[1] = 0.0434687f;
        table6_lin[2] = 0.08694739f;
        table6_lin[3] = 0.1304261f;
        table6_lin[4] = 0.1739048f;
        table6_lin[5] = 0.2173835f;
        table6_lin[6] = 0.2608622f;
        table6_lin[7] = 0.3043409f;
        table6_lin[8] = 0.3478196f;
        table6_lin[9] = 0.3912983f;
        table6_lin[10] = 0.434777f;
        table6_lin[11] = 0.4782557f;
        table6_lin[12] = 0.5217344f;
        table6_lin[13] = 0.565213f;
        table6_lin[14] = 0.6086918f;
        table6_lin[15] = 0.6521704f;
        table6_lin[16] = 0.6956491f;
        table6_lin[17] = 0.7391278f;
        table6_lin[18] = 0.7826065f;
        table6_lin[19] = 0.8260852f;
        table6_lin[20] = 0.8695639f;
        table6_lin[21] = 0.9130426f;
        table6_lin[22] = 0.9565213f;
        table6_lin[23] = 0.999999f;

        // T8 (lines 403-445)
        for (int i = 0; i < 11; ++i)
            table8_lin[i] = 0.002257127f;

        table8_lin[11] = 0.000807641f;
        table8_lin[12] = 0.0002590034f;
        table8_lin[13] = 0.0001466583f;
        table8_lin[14] = 0.000105361f;
        table8_lin[15] = 0.00008688696f;
        table8_lin[16] = 0.00007712693f;
        table8_lin[17] = 0.00007082194f;
        table8_lin[18] = 0.00006535164f;
        table8_lin[19] = 0.00005942077f;
        table8_lin[20] = 0.00005248035f;
        table8_lin[21] = 0.00004474115f;
        table8_lin[22] = 0.00003699339f;
        table8_lin[23] = 0.00002985739f;
        table8_lin[24] = 0.00002377450f;
        table8_lin[25] = 0.00001915936f;
        table8_lin[26] = 0.00001565825f;
        table8_lin[27] = 0.00001302099f;
        table8_lin[28] = 0.00001102717f;
        table8_lin[29] = 0.000009554097f;
        table8_lin[30] = 0.000008418394f;
        table8_lin[31] = 0.000007519858f;
        table8_lin[32] = 0.000006788958f;
        table8_lin[33] = 0.000006188009f;
        table8_lin[34] = 0.000005677829f;
        table8_lin[35] = 0.000005232528f;
        table8_lin[36] = 0.000004838532f;
        table8_lin[37] = 0.000004491788f;
        table8_lin[38] = 0.000004193505f;
        table8_lin[39] = 0.000003938723f;
        table8_lin[40] = 0.000003725471f;
        table8_lin[41] = 0.000003552736f;
        table8_lin[42] = 0.000003421820f;
        table8_lin[43] = 0.000003326748f;
        table8_lin[44] = 0.000003267550f;
        table8_lin[45] = 0.000003245660f;

        // T9 (lines 448-472)
        table9_lin[0] = 0.00004848326f;
        table9_lin[1] = 0.00004848326f;
        table9_lin[2] = 0.00004848326f;
        table9_lin[3] = 0.00004188835f;
        table9_lin[4] = 0.00002785662f;
        table9_lin[5] = 0.00001560057f;
        table9_lin[6] = 0.00001201397f;
        table9_lin[7] = 0.000008427365f;
        table9_lin[8] = 0.000005328864f;
        table9_lin[9] = 0.000004453937f;
        table9_lin[10] = 0.000003579009f;
        table9_lin[11] = 0.000002704082f;
        table9_lin[12] = 0.000002101815f;
        table9_lin[13] = 0.000001772209f;
        table9_lin[14] = 0.000001442603f;
        table9_lin[15] = 0.000001112997f;
        table9_lin[16] = 8.481028e-7f;
        table9_lin[17] = 6.281776e-7f;
        table9_lin[18] = 4.082524e-7f;
        table9_lin[19] = 2.060375e-7f;
        table9_lin[20] = 1.126142e-7f;
        table9_lin[21] = 1.919095e-8f;
        table9_lin[22] = 3.280834e-10f;
        table9_lin[23] = -4.332800e-9f;

        // T10 (lines 475-489)
        table10_exp[0] = 0.8766871f;
        table10_exp[1] = 0.8766871f;
        table10_exp[2] = 0.9343757f;
        table10_exp[3] = 0.966794f;
        table10_exp[4] = 0.9838194f;
        table10_exp[5] = 0.9926132f;
        table10_exp[6] = 0.9970101f;
        table10_exp[7] = 0.9992085f;

        for (int i = 8; i < 25; ++i)
            table10_exp[i] = 1.0f;

        // T12 (lines 494-542)
        table12_exp_neg[0] = 0.000002987261f;
        table12_exp_neg[1] = 0.000005974523f;
        table12_exp_neg[2] = 0.00001194905f;
        table12_exp_neg[3] = 0.00002389809f;
        table12_exp_neg[4] = 0.00004779618f;
        table12_exp_neg[5] = 0.00009559237f;
        table12_exp_neg[6] = 0.0001911847f;
        table12_exp_neg[7] = 0.0003823695f;
        table12_exp_neg[8] = 0.0007647389f;
        table12_exp_neg[9] = 0.001529478f;
        table12_exp_neg[10] = 0.003058956f;
        table12_exp_neg[11] = 0.006117912f;
        table12_exp_neg[12] = 0.01223582f;
        table12_exp_neg[13] = 0.02447165f;
        table12_exp_neg[14] = 0.04894329f;
        table12_exp_neg[15] = 0.09788658f;
        table12_exp_neg[16] = 0.1957732f;
        table12_exp_neg[17] = 0.3915463f;
        table12_exp_neg[18] = 0.7830927f;
        table12_exp_neg[19] = 1.0f;
        table12_exp_neg[20] = 1.0f;
        table12_exp_neg[21] = 1.0f;
        table12_exp_neg[22] = 1.0f;
        table12_exp_neg[23] = 1.0f;
        table12_exp_neg[24] = 1.0f;
        table12_exp_neg[25] = 1.0f;
        table12_exp_neg[26] = 1.0f;
        table12_exp_neg[27] = 1.0f;
        table12_exp_neg[28] = 1.0f;
        table12_exp_neg[29] = 0.7810927f;
        table12_exp_neg[30] = 0.3895463f;
        table12_exp_neg[31] = 0.1937732f;
        table12_exp_neg[32] = 0.09588659f;
        table12_exp_neg[33] = 0.04694329f;
        table12_exp_neg[34] = 0.02247165f;
        table12_exp_neg[35] = 0.01023582f;
        table12_exp_neg[36] = 0.004117912f;
        table12_exp_neg[37] = 0.001058956f;
        table12_exp_neg[38] = 0.0f;
        table12_exp_neg[39] = 0.0f;
        table12_exp_neg[40] = 0.0f;
        table12_exp_neg[41] = 0.0f;
        table12_exp_neg[42] = 0.0f;
        table12_exp_neg[43] = 0.0f;
        table12_exp_neg[44] = 0.0f;
        table12_exp_neg[45] = 0.0f;
        table12_exp_neg[46] = 0.0f;
        table12_exp_neg[47] = 0.0f;

        // T13 (lines 545-650)
        table13_lin[0] = 0.0f;
        table13_lin[1] = 0.002895139f;
        table13_lin[2] = 0.01001967f;
        table13_lin[3] = 0.01859283f;
        table13_lin[4] = 0.0278201f;
        table13_lin[5] = 0.03739988f;
        table13_lin[6] = 0.04719125f;
        table13_lin[7] = 0.05711957f;
        table13_lin[8] = 0.06714159f;
        table13_lin[9] = 0.07723056f;
        table13_lin[10] = 0.087369f;
        table13_lin[11] = 0.09754504f;
        table13_lin[12] = 0.1077503f;
        table13_lin[13] = 0.1179788f;
        table13_lin[14] = 0.1282261f;
        table13_lin[15] = 0.1384886f;
        table13_lin[16] = 0.1487638f;
        table13_lin[17] = 0.1590496f;
        table13_lin[18] = 0.1693444f;
        table13_lin[19] = 0.1796468f;
        table13_lin[20] = 0.1899559f;
        table13_lin[21] = 0.2002707f;
        table13_lin[22] = 0.2105905f;
        table13_lin[23] = 0.2209146f;
        table13_lin[24] = 0.2312426f;
        table13_lin[25] = 0.2415741f;
        table13_lin[26] = 0.2519086f;
        table13_lin[27] = 0.2622458f;
        table13_lin[28] = 0.2725855f;
        table13_lin[29] = 0.2829274f;
        table13_lin[30] = 0.2932713f;
        table13_lin[31] = 0.303617f;
        table13_lin[32] = 0.3139643f;
        table13_lin[33] = 0.3243132f;
        table13_lin[34] = 0.3346635f;
        table13_lin[35] = 0.345015f;
        table13_lin[36] = 0.3553677f;
        table13_lin[37] = 0.3657215f;
        table13_lin[38] = 0.3760763f;
        table13_lin[39] = 0.3864319f;
        table13_lin[40] = 0.3967885f;
        table13_lin[41] = 0.4071458f;
        table13_lin[42] = 0.4175039f;
        table13_lin[43] = 0.4278626f;
        table13_lin[44] = 0.4382221f;
        table13_lin[45] = 0.4485821f;
        table13_lin[46] = 0.4589426f;
        table13_lin[47] = 0.4693037f;
        table13_lin[48] = 0.4796653f;
        table13_lin[49] = 0.4900274f;
        table13_lin[50] = 0.5003899f;
        table13_lin[51] = 0.5107529f;
        table13_lin[52] = 0.5211161f;
        table13_lin[53] = 0.5314798f;
        table13_lin[54] = 0.5418439f;
        table13_lin[55] = 0.5522082f;
        table13_lin[56] = 0.562573f;
        table13_lin[57] = 0.5729379f;
        table13_lin[58] = 0.5833032f;
        table13_lin[59] = 0.5936688f;
        table13_lin[60] = 0.6040345f;
        table13_lin[61] = 0.6144006f;
        table13_lin[62] = 0.6247668f;
        table13_lin[63] = 0.6351333f;
        table13_lin[64] = 0.6455f;
        table13_lin[65] = 0.6558669f;
        table13_lin[66] = 0.666234f;
        table13_lin[67] = 0.6766013f;
        table13_lin[68] = 0.6869688f;
        table13_lin[69] = 0.6973364f;
        table13_lin[70] = 0.7077042f;
        table13_lin[71] = 0.7180721f;
        table13_lin[72] = 0.7284402f;
        table13_lin[73] = 0.7388085f;
        table13_lin[74] = 0.7491769f;
        table13_lin[75] = 0.7595453f;
        table13_lin[76] = 0.769914f;
        table13_lin[77] = 0.7802827f;
        table13_lin[78] = 0.7906516f;
        table13_lin[79] = 0.8010206f;
        table13_lin[80] = 0.8113897f;
        table13_lin[81] = 0.8217589f;
        table13_lin[82] = 0.8321282f;
        table13_lin[83] = 0.8424976f;
        table13_lin[84] = 0.8528671f;
        table13_lin[85] = 0.8632367f;
        table13_lin[86] = 0.8736063f;
        table13_lin[87] = 0.883976f;
        table13_lin[88] = 0.8943459f;
        table13_lin[89] = 0.9047158f;
        table13_lin[90] = 0.9150858f;
        table13_lin[91] = 0.9254559f;
        table13_lin[92] = 0.935826f;
        table13_lin[93] = 0.9461962f;
        table13_lin[94] = 0.9565665f;
        table13_lin[95] = 0.9669368f;
        table13_lin[96] = 0.9773072f;
        table13_lin[97] = 0.9876777f;
        table13_lin[98] = 0.9980482f;

        for (int i = 99; i < 252; ++i)
            table13_lin[i] = 1.0f;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CL1BCompressor)
};
