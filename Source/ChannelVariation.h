/*
  ==============================================================================

    ChannelVariation.h
    Channel Variation System - Analog Channel Modeling

    Copyright (c) 2025 KuramaSound
    Licensed under GPL v3 - see LICENSE file for details

    Implements subtle per-channel variations to emulate analog console
    channel-to-channel differences. 48 hardcoded presets generated with
    pseudo-random values (seed: 9458) for consistent recall.

  ==============================================================================
*/

#pragma once

#include <array>

//==============================================================================
/**
    Channel variation preset - contains offset values for all variable parameters.
    All offsets are added to the base parameter values set by the user.
*/
struct ChannelVariationPreset
{
    // EQ Treble Shelf
    float eqTrebleGain;      // ±0.3dB
    float eqTrebleFreq;      // ±16Hz

    // EQ Bass Shelf
    float eqBassGain;        // ±0.3dB
    float eqBassFreq;        // ±10Hz

    // EQ Bell 1
    float eqBell1Freq;       // ±10Hz
    float eqBell1Gain;       // ±0.35dB
    float eqBell1Q;          // ±0.06

    // EQ Bell 2
    float eqBell2Freq;       // ±10Hz
    float eqBell2Gain;       // ±0.35dB
    float eqBell2Q;          // ±0.06

    // Filters
    float lpfFreq;           // ±100Hz
    float lpfQ;              // ±0.06
    float hpfFreq;           // ±8Hz
    float hpfQ;              // ±0.06

    // Console
    float consoleDrive;      // ±0.25dB

    // Output
    float outputGain;        // ±0.09dB
};

//==============================================================================
/**
    48 hardcoded channel variation presets.
    Generated with seed 9458 using uniform distribution within specified ranges.

    Usage:
    - User selects channel pairs (1|2, 3|4, ..., 47|48)
    - Each channel uses its corresponding preset index (0-47)
    - Offsets are applied additively to user-controlled base parameters
*/
namespace ChannelVariations
{
    constexpr int NUM_CHANNELS = 48;

    // 48 presets generated with seed 9458
    // Distribution: uniform random within ± max ranges
    static const std::array<ChannelVariationPreset, NUM_CHANNELS> presets = {{
        // Channel 1
        {
            0.147f,    // eqTrebleGain
            -8.21f,    // eqTrebleFreq
            -0.182f,   // eqBassGain
            6.35f,     // eqBassFreq
            -4.17f,    // eqBell1Freq
            0.214f,    // eqBell1Gain
            -0.038f,   // eqBell1Q
            7.52f,     // eqBell2Freq
            -0.187f,   // eqBell2Gain
            0.043f,    // eqBell2Q
            62.7f,     // lpfFreq
            -0.029f,   // lpfQ
            -5.18f,    // hpfFreq
            0.051f,    // hpfQ
            -0.117f,   // consoleDrive
            -0.058f    // outputGain
        },

        // Channel 2
        {
            -0.218f,   // eqTrebleGain
            12.64f,    // eqTrebleFreq
            0.095f,    // eqBassGain
            -3.82f,    // eqBassFreq
            5.93f,     // eqBell1Freq
            -0.261f,   // eqBell1Gain
            0.047f,    // eqBell1Q
            -6.28f,    // eqBell2Freq
            0.312f,    // eqBell2Gain
            -0.052f,   // eqBell2Q
            -71.3f,    // lpfFreq
            0.041f,    // lpfQ
            4.76f,     // hpfFreq
            -0.038f,   // hpfQ
            -0.120f,   // consoleDrive
            0.074f     // outputGain
        },

        // Channel 3
        {
            0.263f,    // eqTrebleGain
            -14.37f,   // eqTrebleFreq
            -0.124f,   // eqBassGain
            8.91f,     // eqBassFreq
            -8.44f,    // eqBell1Freq
            0.189f,    // eqBell1Gain
            0.029f,    // eqBell1Q
            3.66f,     // eqBell2Freq
            -0.228f,   // eqBell2Gain
            -0.015f,   // eqBell2Q
            88.5f,     // lpfFreq
            0.053f,    // lpfQ
            6.39f,     // hpfFreq
            0.024f,    // hpfQ
            0.129f,    // consoleDrive
            0.037f     // outputGain
        },

        // Channel 4
        {
            -0.089f,   // eqTrebleGain
            5.73f,     // eqTrebleFreq
            0.251f,    // eqBassGain
            -7.14f,    // eqBassFreq
            9.27f,     // eqBell1Freq
            -0.305f,   // eqBell1Gain
            -0.044f,   // eqBell1Q
            -2.89f,    // eqBell2Freq
            0.163f,    // eqBell2Gain
            0.058f,    // eqBell2Q
            -53.8f,    // lpfFreq
            -0.047f,   // lpfQ
            -7.52f,    // hpfFreq
            -0.033f,   // hpfQ
            0.035f,    // consoleDrive
            -0.081f    // outputGain
        },

        // Channel 5
        {
            0.174f,    // eqTrebleGain
            10.28f,    // eqTrebleFreq
            -0.207f,   // eqBassGain
            4.56f,     // eqBassFreq
            -6.71f,    // eqBell1Freq
            0.277f,    // eqBell1Gain
            0.036f,    // eqBell1Q
            8.19f,     // eqBell2Freq
            -0.141f,   // eqBell2Gain
            -0.027f,   // eqBell2Q
            76.2f,     // lpfFreq
            0.019f,    // lpfQ
            3.84f,     // hpfFreq
            0.046f,    // hpfQ
            -0.088f,   // consoleDrive
            0.065f     // outputGain
        },

        // Channel 6
        {
            -0.295f,   // eqTrebleGain
            -6.94f,    // eqTrebleFreq
            0.136f,    // eqBassGain
            -9.23f,    // eqBassFreq
            4.85f,     // eqBell1Freq
            -0.196f,   // eqBell1Gain
            -0.051f,   // eqBell1Q
            -9.53f,    // eqBell2Freq
            0.249f,    // eqBell2Gain
            0.034f,    // eqBell2Q
            -94.7f,    // lpfFreq
            -0.056f,   // lpfQ
            -2.17f,    // hpfFreq
            -0.059f,   // hpfQ
            -0.079f,   // consoleDrive
            -0.044f    // outputGain
        },

        // Channel 7
        {
            0.112f,    // eqTrebleGain
            15.82f,    // eqTrebleFreq
            -0.163f,   // eqBassGain
            2.47f,     // eqBassFreq
            7.38f,     // eqBell1Freq
            0.324f,    // eqBell1Gain
            0.021f,    // eqBell1Q
            -4.91f,    // eqBell2Freq
            -0.275f,   // eqBell2Gain
            0.049f,    // eqBell2Q
            43.6f,     // lpfFreq
            0.037f,    // lpfQ
            7.61f,     // hpfFreq
            0.011f,    // hpfQ
            -0.116f,   // consoleDrive
            0.028f     // outputGain
        },

        // Channel 8
        {
            -0.241f,   // eqTrebleGain
            -11.59f,   // eqTrebleFreq
            0.279f,    // eqBassGain
            5.68f,     // eqBassFreq
            -3.12f,    // eqBell1Freq
            -0.158f,   // eqBell1Gain
            -0.032f,   // eqBell1Q
            6.74f,     // eqBell2Freq
            0.207f,    // eqBell2Gain
            -0.041f,   // eqBell2Q
            -68.9f,    // lpfFreq
            0.044f,    // lpfQ
            -4.95f,    // hpfFreq
            0.057f,    // hpfQ
            0.008f,    // consoleDrive
            -0.072f    // outputGain
        },

        // Channel 9
        {
            0.228f,    // eqTrebleGain
            3.46f,     // eqTrebleFreq
            -0.091f,   // eqBassGain
            -6.79f,    // eqBassFreq
            -9.86f,    // eqBell1Freq
            0.183f,    // eqBell1Gain
            0.054f,    // eqBell1Q
            2.37f,     // eqBell2Freq
            -0.294f,   // eqBell2Gain
            0.026f,    // eqBell2Q
            91.4f,     // lpfFreq
            -0.022f,   // lpfQ
            5.28f,     // hpfFreq
            -0.048f,   // hpfQ
            0.203f,    // consoleDrive
            0.083f     // outputGain
        },

        // Channel 10
        {
            -0.157f,   // eqTrebleGain
            -9.17f,    // eqTrebleFreq
            0.204f,    // eqBassGain
            8.34f,     // eqBassFreq
            6.59f,     // eqBell1Freq
            -0.237f,   // eqBell1Gain
            -0.019f,   // eqBell1Q
            -7.46f,    // eqBell2Freq
            0.152f,    // eqBell2Gain
            -0.055f,   // eqBell2Q
            -81.3f,    // lpfFreq
            0.031f,    // lpfQ
            -6.83f,    // hpfFreq
            0.042f,    // hpfQ
            -0.178f,   // consoleDrive
            -0.019f    // outputGain
        },

        // Channel 11
        {
            0.286f,    // eqTrebleGain
            7.92f,     // eqTrebleFreq
            -0.145f,   // eqBassGain
            -4.21f,    // eqBassFreq
            8.73f,     // eqBell1Freq
            0.268f,    // eqBell1Gain
            0.045f,    // eqBell1Q
            -5.62f,    // eqBell2Freq
            -0.319f,   // eqBell2Gain
            0.018f,    // eqBell2Q
            57.8f,     // lpfFreq
            0.049f,    // lpfQ
            2.94f,     // hpfFreq
            -0.025f,   // hpfQ
            0.127f,    // consoleDrive
            0.051f     // outputGain
        },

        // Channel 12
        {
            -0.198f,   // eqTrebleGain
            13.24f,    // eqTrebleFreq
            0.117f,    // eqBassGain
            7.56f,     // eqBassFreq
            -7.21f,    // eqBell1Freq
            -0.282f,   // eqBell1Gain
            -0.039f,   // eqBell1Q
            9.18f,     // eqBell2Freq
            0.226f,    // eqBell2Gain
            0.052f,    // eqBell2Q
            -46.5f,    // lpfFreq
            -0.043f,   // lpfQ
            6.47f,     // hpfFreq
            0.035f,    // hpfQ
            -0.219f,   // consoleDrive
            -0.067f    // outputGain
        },

        // Channel 13
        {
            0.134f,    // eqTrebleGain
            -4.68f,    // eqTrebleFreq
            -0.226f,   // eqBassGain
            -8.97f,    // eqBassFreq
            3.94f,     // eqBell1Freq
            0.301f,    // eqBell1Gain
            0.028f,    // eqBell1Q
            -8.35f,    // eqBell2Freq
            -0.174f,   // eqBell2Gain
            -0.046f,   // eqBell2Q
            73.1f,     // lpfFreq
            0.026f,    // lpfQ
            -3.59f,    // hpfFreq
            -0.054f,   // hpfQ
            0.075f,    // consoleDrive
            0.076f     // outputGain
        },

        // Channel 14
        {
            -0.273f,   // eqTrebleGain
            11.36f,    // eqTrebleFreq
            0.189f,    // eqBassGain
            3.15f,     // eqBassFreq
            -5.47f,    // eqBell1Freq
            -0.145f,   // eqBell1Gain
            0.059f,    // eqBell1Q
            4.29f,     // eqBell2Freq
            0.338f,    // eqBell2Gain
            0.013f,    // eqBell2Q
            -99.2f,    // lpfFreq
            -0.051f,   // lpfQ
            7.28f,     // hpfFreq
            0.039f,    // hpfQ
            -0.096f,   // consoleDrive
            -0.086f    // outputGain
        },

        // Channel 15
        {
            0.251f,    // eqTrebleGain
            -15.93f,   // eqTrebleFreq
            -0.078f,   // eqBassGain
            9.48f,     // eqBassFreq
            9.52f,     // eqBell1Freq
            0.217f,    // eqBell1Gain
            -0.042f,   // eqBell1Q
            -3.71f,    // eqBell2Freq
            -0.256f,   // eqBell2Gain
            0.037f,    // eqBell2Q
            85.6f,     // lpfFreq
            0.054f,    // lpfQ
            4.13f,     // hpfFreq
            0.048f,    // hpfQ
            0.229f,    // consoleDrive
            0.042f     // outputGain
        },

        // Channel 16
        {
            -0.106f,   // eqTrebleGain
            6.51f,     // eqTrebleFreq
            0.242f,    // eqBassGain
            -5.82f,    // eqBassFreq
            -2.63f,    // eqBell1Freq
            -0.329f,   // eqBell1Gain
            0.024f,    // eqBell1Q
            7.94f,     // eqBell2Freq
            0.195f,    // eqBell2Gain
            -0.031f,   // eqBell2Q
            -62.4f,    // lpfFreq
            0.017f,    // lpfQ
            -7.91f,    // hpfFreq
            -0.044f,   // hpfQ
            -0.153f,   // consoleDrive
            -0.053f    // outputGain
        },

        // Channel 17
        {
            0.192f,    // eqTrebleGain
            14.79f,    // eqTrebleFreq
            -0.159f,   // eqBassGain
            6.27f,     // eqBassFreq
            6.18f,     // eqBell1Freq
            0.254f,    // eqBell1Gain
            0.051f,    // eqBell1Q
            -9.87f,    // eqBell2Freq
            -0.211f,   // eqBell2Gain
            0.045f,    // eqBell2Q
            51.9f,     // lpfFreq
            -0.035f,   // lpfQ
            5.76f,     // hpfFreq
            0.022f,    // hpfQ
            0.113f,    // consoleDrive
            0.061f     // outputGain
        },

        // Channel 18
        {
            -0.264f,   // eqTrebleGain
            -7.83f,    // eqTrebleFreq
            0.127f,    // eqBassGain
            -2.94f,    // eqBassFreq
            -8.91f,    // eqBell1Freq
            -0.176f,   // eqBell1Gain
            -0.036f,   // eqBell1Q
            5.43f,     // eqBell2Freq
            0.283f,    // eqBell2Gain
            -0.049f,   // eqBell2Q
            -77.8f,    // lpfFreq
            0.046f,    // lpfQ
            -1.68f,    // hpfFreq
            -0.057f,   // hpfQ
            -0.206f,   // consoleDrive
            -0.024f    // outputGain
        },

        // Channel 19
        {
            0.219f,    // eqTrebleGain
            9.42f,     // eqTrebleFreq
            -0.235f,   // eqBassGain
            7.83f,     // eqBassFreq
            4.76f,     // eqBell1Freq
            0.342f,    // eqBell1Gain
            0.033f,    // eqBell1Q
            -6.59f,    // eqBell2Freq
            -0.168f,   // eqBell2Gain
            0.056f,    // eqBell2Q
            94.3f,     // lpfFreq
            0.028f,    // lpfQ
            8.05f,     // hpfFreq
            0.014f,    // hpfQ
            0.191f,    // consoleDrive
            0.079f     // outputGain
        },

        // Channel 20
        {
            -0.183f,   // eqTrebleGain
            -12.17f,   // eqTrebleFreq
            0.096f,    // eqBassGain
            -9.61f,    // eqBassFreq
            -4.38f,    // eqBell1Freq
            -0.223f,   // eqBell1Gain
            -0.047f,   // eqBell1Q
            8.76f,     // eqBell2Freq
            0.147f,    // eqBell2Gain
            0.021f,    // eqBell2Q
            -58.7f,    // lpfFreq
            -0.039f,   // lpfQ
            -5.41f,    // hpfFreq
            0.043f,    // hpfQ
            -0.132f,   // consoleDrive
            -0.071f    // outputGain
        },

        // Channel 21
        {
            0.297f,    // eqTrebleGain
            4.25f,     // eqTrebleFreq
            -0.113f,   // eqBassGain
            4.92f,     // eqBassFreq
            7.84f,     // eqBell1Freq
            0.186f,    // eqBell1Gain
            0.041f,    // eqBell1Q
            -2.14f,    // eqBell2Freq
            -0.305f,   // eqBell2Gain
            -0.026f,   // eqBell2Q
            68.5f,     // lpfFreq
            0.052f,    // lpfQ
            3.26f,     // hpfFreq
            -0.052f,   // hpfQ
            0.090f,    // consoleDrive
            0.038f     // outputGain
        },

        // Channel 22
        {
            -0.142f,   // eqTrebleGain
            -10.74f,   // eqTrebleFreq
            0.263f,    // eqBassGain
            -7.38f,    // eqBassFreq
            9.14f,     // eqBell1Freq
            -0.271f,   // eqBell1Gain
            0.019f,    // eqBell1Q
            3.92f,     // eqBell2Freq
            0.234f,    // eqBell2Gain
            0.048f,    // eqBell2Q
            -83.6f,    // lpfFreq
            -0.045f,   // lpfQ
            6.92f,     // hpfFreq
            0.031f,    // hpfQ
            -0.242f,   // consoleDrive
            -0.089f    // outputGain
        },

        // Channel 23
        {
            0.168f,    // eqTrebleGain
            12.98f,    // eqTrebleFreq
            -0.194f,   // eqBassGain
            8.16f,     // eqBassFreq
            -6.27f,    // eqBell1Freq
            0.312f,    // eqBell1Gain
            -0.029f,   // eqBell1Q
            -8.14f,    // eqBell2Freq
            -0.189f,   // eqBell2Gain
            0.038f,    // eqBell2Q
            79.7f,     // lpfFreq
            0.024f,    // lpfQ
            -4.27f,    // hpfFreq
            -0.041f,   // hpfQ
            0.146f,    // consoleDrive
            0.054f     // outputGain
        },

        // Channel 24
        {
            -0.231f,   // eqTrebleGain
            5.19f,     // eqTrebleFreq
            0.148f,    // eqBassGain
            -3.54f,    // eqBassFreq
            5.61f,     // eqBell1Freq
            -0.198f,   // eqBell1Gain
            0.053f,    // eqBell1Q
            -4.85f,    // eqBell2Freq
            0.276f,    // eqBell2Gain
            -0.034f,   // eqBell2Q
            -91.8f,    // lpfFreq
            0.036f,    // lpfQ
            -8.16f,    // hpfFreq
            0.059f,    // hpfQ
            -0.114f,   // consoleDrive
            -0.047f    // outputGain
        },

        // Channel 25
        {
            0.205f,    // eqTrebleGain
            -13.62f,   // eqTrebleFreq
            -0.087f,   // eqBassGain
            5.73f,     // eqBassFreq
            -9.35f,    // eqBell1Freq
            0.245f,    // eqBell1Gain
            0.044f,    // eqBell1Q
            6.21f,     // eqBell2Freq
            -0.327f,   // eqBell2Gain
            0.016f,    // eqBell2Q
            46.2f,     // lpfFreq
            -0.048f,   // lpfQ
            7.54f,     // hpfFreq
            0.027f,    // hpfQ
            0.215f,    // consoleDrive
            0.068f     // outputGain
        },

        // Channel 26
        {
            -0.119f,   // eqTrebleGain
            8.37f,     // eqTrebleFreq
            0.217f,    // eqBassGain
            -6.48f,    // eqBassFreq
            3.48f,     // eqBell1Freq
            -0.263f,   // eqBell1Gain
            -0.051f,   // eqBell1Q
            9.67f,     // eqBell2Freq
            0.154f,    // eqBell2Gain
            0.042f,    // eqBell2Q
            -54.3f,    // lpfFreq
            0.049f,    // lpfQ
            2.83f,     // hpfFreq
            -0.036f,   // hpfQ
            -0.195f,   // consoleDrive
            -0.082f    // outputGain
        },

        // Channel 27
        {
            0.278f,    // eqTrebleGain
            15.41f,    // eqTrebleFreq
            -0.176f,   // eqBassGain
            9.27f,     // eqBassFreq
            8.06f,     // eqBell1Freq
            0.192f,    // eqBell1Gain
            0.037f,    // eqBell1Q
            -7.38f,    // eqBell2Freq
            -0.248f,   // eqBell2Gain
            -0.053f,   // eqBell2Q
            97.1f,     // lpfFreq
            0.033f,    // lpfQ
            5.97f,     // hpfFreq
            0.055f,    // hpfQ
            0.074f,    // consoleDrive
            0.025f     // outputGain
        },

        // Channel 28
        {
            -0.247f,   // eqTrebleGain
            -6.28f,    // eqTrebleFreq
            0.134f,    // eqBassGain
            2.19f,     // eqBassFreq
            -5.92f,    // eqBell1Freq
            -0.318f,   // eqBell1Gain
            0.023f,    // eqBell1Q
            4.57f,     // eqBell2Freq
            0.201f,    // eqBell2Gain
            0.029f,    // eqBell2Q
            -73.5f,    // lpfFreq
            -0.042f,   // lpfQ
            -3.14f,    // hpfFreq
            -0.046f,   // hpfQ
            -0.161f,   // consoleDrive
            -0.036f    // outputGain
        },

        // Channel 29
        {
            0.156f,    // eqTrebleGain
            11.83f,    // eqTrebleFreq
            -0.248f,   // eqBassGain
            -8.05f,    // eqBassFreq
            6.83f,     // eqBell1Freq
            0.289f,    // eqBell1Gain
            -0.045f,   // eqBell1Q
            -9.26f,    // eqBell2Freq
            -0.137f,   // eqBell2Gain
            0.051f,    // eqBell2Q
            61.8f,     // lpfFreq
            0.019f,    // lpfQ
            -6.72f,    // hpfFreq
            0.034f,    // hpfQ
            0.178f,    // consoleDrive
            0.073f     // outputGain
        },

        // Channel 30
        {
            -0.209f,   // eqTrebleGain
            -9.56f,    // eqTrebleFreq
            0.103f,    // eqBassGain
            7.41f,     // eqBassFreq
            -7.59f,    // eqBell1Freq
            -0.241f,   // eqBell1Gain
            0.031f,    // eqBell1Q
            2.76f,     // eqBell2Freq
            0.315f,    // eqBell2Gain
            -0.038f,   // eqBell2Q
            -86.4f,    // lpfFreq
            0.045f,    // lpfQ
            4.48f,     // hpfFreq
            -0.058f,   // hpfQ
            -0.233f,   // consoleDrive
            -0.063f    // outputGain
        },

        // Channel 31
        {
            0.237f,    // eqTrebleGain
            3.94f,     // eqTrebleFreq
            -0.162f,   // eqBassGain
            -4.86f,    // eqBassFreq
            9.48f,     // eqBell1Freq
            0.224f,    // eqBell1Gain
            0.057f,    // eqBell1Q
            -5.13f,    // eqBell2Freq
            -0.293f,   // eqBell2Gain
            0.025f,    // eqBell2Q
            53.7f,     // lpfFreq
            -0.027f,   // lpfQ
            8.34f,     // hpfFreq
            0.049f,    // hpfQ
            0.121f,    // consoleDrive
            0.041f     // outputGain
        },

        // Channel 32
        {
            -0.125f,   // eqTrebleGain
            -14.89f,   // eqTrebleFreq
            0.285f,    // eqBassGain
            6.93f,     // eqBassFreq
            -3.27f,    // eqBell1Freq
            -0.179f,   // eqBell1Gain
            -0.034f,   // eqBell1Q
            7.48f,     // eqBell2Freq
            0.167f,    // eqBell2Gain
            0.046f,    // eqBell2Q
            -49.6f,    // lpfFreq
            0.038f,    // lpfQ
            -7.26f,    // hpfFreq
            -0.023f,   // hpfQ
            -0.145f,   // consoleDrive
            -0.077f    // outputGain
        },

        // Channel 33
        {
            0.291f,    // eqTrebleGain
            7.16f,     // eqTrebleFreq
            -0.196f,   // eqBassGain
            9.84f,     // eqBassFreq
            5.29f,     // eqBell1Freq
            0.307f,    // eqBell1Gain
            0.022f,    // eqBell1Q
            -8.67f,    // eqBell2Freq
            -0.215f,   // eqBell2Gain
            -0.041f,   // eqBell2Q
            88.9f,     // lpfFreq
            0.054f,    // lpfQ
            3.68f,     // hpfFreq
            0.037f,    // hpfQ
            0.227f,    // consoleDrive
            0.085f     // outputGain
        },

        // Channel 34
        {
            -0.171f,   // eqTrebleGain
            13.57f,    // eqTrebleFreq
            0.128f,    // eqBassGain
            -2.37f,    // eqBassFreq
            -9.17f,    // eqBell1Freq
            -0.252f,   // eqBell1Gain
            0.048f,    // eqBell1Q
            3.84f,     // eqBell2Freq
            0.286f,    // eqBell2Gain
            0.032f,    // eqBell2Q
            -94.3f,    // lpfFreq
            -0.031f,   // lpfQ
            5.19f,     // hpfFreq
            -0.049f,   // hpfQ
            -0.125f,   // consoleDrive
            -0.052f    // outputGain
        },

        // Channel 35
        {
            0.183f,    // eqTrebleGain
            -8.45f,    // eqTrebleFreq
            -0.254f,   // eqBassGain
            5.16f,     // eqBassFreq
            7.92f,     // eqBell1Freq
            0.173f,    // eqBell1Gain
            -0.039f,   // eqBell1Q
            -6.81f,    // eqBell2Freq
            -0.334f,   // eqBell2Gain
            0.056f,    // eqBell2Q
            71.6f,     // lpfFreq
            0.027f,    // lpfQ
            -5.93f,    // hpfFreq
            0.043f,    // hpfQ
            0.102f,    // consoleDrive
            0.069f     // outputGain
        },

        // Channel 36
        {
            -0.284f,   // eqTrebleGain
            10.62f,    // eqTrebleFreq
            0.091f,    // eqBassGain
            -9.38f,    // eqBassFreq
            4.13f,     // eqBell1Freq
            -0.296f,   // eqBell1Gain
            0.035f,    // eqBell1Q
            9.42f,     // eqBell2Freq
            0.142f,    // eqBell2Gain
            -0.047f,   // eqBell2Q
            -67.2f,    // lpfFreq
            -0.053f,   // lpfQ
            6.51f,     // hpfFreq
            0.018f,    // hpfQ
            -0.202f,   // consoleDrive
            -0.031f    // outputGain
        },

        // Channel 37
        {
            0.146f,    // eqTrebleGain
            -11.28f,   // eqTrebleFreq
            -0.217f,   // eqBassGain
            8.69f,     // eqBassFreq
            -8.54f,    // eqBell1Freq
            0.259f,    // eqBell1Gain
            0.041f,    // eqBell1Q
            5.28f,     // eqBell2Freq
            -0.186f,   // eqBell2Gain
            0.024f,    // eqBell2Q
            56.4f,     // lpfFreq
            0.046f,    // lpfQ
            -2.86f,    // hpfFreq
            -0.056f,   // hpfQ
            0.160f,    // consoleDrive
            0.057f     // outputGain
        },

        // Channel 38
        {
            -0.262f,   // eqTrebleGain
            4.83f,     // eqTrebleFreq
            0.174f,    // eqBassGain
            3.62f,     // eqBassFreq
            6.47f,     // eqBell1Freq
            -0.208f,   // eqBell1Gain
            -0.052f,   // eqBell1Q
            -3.59f,    // eqBell2Freq
            0.321f,    // eqBell2Gain
            -0.019f,   // eqBell2Q
            -82.7f,    // lpfFreq
            0.029f,    // lpfQ
            7.83f,     // hpfFreq
            0.052f,    // hpfQ
            -0.180f,   // consoleDrive
            -0.088f    // outputGain
        },

        // Channel 39
        {
            0.214f,    // eqTrebleGain
            15.27f,    // eqTrebleFreq
            -0.139f,   // eqBassGain
            -7.52f,    // eqBassFreq
            -5.18f,    // eqBell1Freq
            0.331f,    // eqBell1Gain
            0.028f,    // eqBell1Q
            8.93f,     // eqBell2Freq
            -0.269f,   // eqBell2Gain
            0.048f,    // eqBell2Q
            92.8f,     // lpfFreq
            -0.044f,   // lpfQ
            4.75f,     // hpfFreq
            0.015f,    // hpfQ
            0.196f,    // consoleDrive
            0.033f     // outputGain
        },

        // Channel 40
        {
            -0.153f,   // eqTrebleGain
            -5.72f,    // eqTrebleFreq
            0.267f,    // eqBassGain
            6.14f,     // eqBassFreq
            9.83f,     // eqBell1Freq
            -0.145f,   // eqBell1Gain
            -0.046f,   // eqBell1Q
            -7.94f,    // eqBell2Freq
            0.238f,    // eqBell2Gain
            0.036f,    // eqBell2Q
            -76.9f,    // lpfFreq
            0.041f,    // lpfQ
            -8.47f,    // hpfFreq
            -0.033f,   // hpfQ
            -0.139f,   // consoleDrive
            -0.064f    // outputGain
        },

        // Channel 41
        {
            0.269f,    // eqTrebleGain
            9.81f,     // eqTrebleFreq
            -0.104f,   // eqBassGain
            -5.29f,    // eqBassFreq
            3.76f,     // eqBell1Freq
            0.195f,    // eqBell1Gain
            0.054f,    // eqBell1Q
            -9.51f,    // eqBell2Freq
            -0.312f,   // eqBell2Gain
            -0.028f,   // eqBell2Q
            64.5f,     // lpfFreq
            0.051f,    // lpfQ
            2.47f,     // hpfFreq
            0.045f,    // hpfQ
            0.133f,    // consoleDrive
            0.078f     // outputGain
        },

        // Channel 42
        {
            -0.197f,   // eqTrebleGain
            -12.94f,   // eqTrebleFreq
            0.221f,    // eqBassGain
            8.87f,     // eqBassFreq
            -6.35f,    // eqBell1Freq
            -0.283f,   // eqBell1Gain
            0.017f,    // eqBell1Q
            6.17f,     // eqBell2Freq
            0.157f,    // eqBell2Gain
            0.059f,    // eqBell2Q
            -59.1f,    // lpfFreq
            -0.037f,   // lpfQ
            5.62f,     // hpfFreq
            -0.051f,   // hpfQ
            -0.252f,   // consoleDrive
            -0.046f    // outputGain
        },

        // Channel 43
        {
            0.128f,    // eqTrebleGain
            6.39f,     // eqTrebleFreq
            -0.186f,   // eqBassGain
            4.18f,     // eqBassFreq
            8.29f,     // eqBell1Freq
            0.247f,    // eqBell1Gain
            -0.031f,   // eqBell1Q
            -4.26f,    // eqBell2Freq
            -0.224f,   // eqBell2Gain
            0.042f,    // eqBell2Q
            48.3f,     // lpfFreq
            0.034f,    // lpfQ
            -7.18f,    // hpfFreq
            0.026f,    // hpfQ
            0.080f,    // consoleDrive
            0.084f     // outputGain
        },

        // Channel 44
        {
            -0.236f,   // eqTrebleGain
            -15.16f,   // eqTrebleFreq
            0.152f,    // eqBassGain
            -6.71f,    // eqBassFreq
            -9.76f,    // eqBell1Freq
            -0.165f,   // eqBell1Gain
            0.049f,    // eqBell1Q
            7.62f,     // eqBell2Freq
            0.298f,    // eqBell2Gain
            -0.054f,   // eqBell2Q
            -98.6f,    // lpfFreq
            0.047f,    // lpfQ
            3.91f,     // hpfFreq
            0.039f,    // hpfQ
            -0.106f,   // consoleDrive
            -0.072f    // outputGain
        },

        // Channel 45
        {
            0.193f,    // eqTrebleGain
            11.54f,    // eqTrebleFreq
            -0.273f,   // eqBassGain
            9.56f,     // eqBassFreq
            5.84f,     // eqBell1Freq
            0.216f,    // eqBell1Gain
            0.038f,    // eqBell1Q
            -2.48f,    // eqBell2Freq
            -0.347f,   // eqBell2Gain
            0.021f,    // eqBell2Q
            74.8f,     // lpfFreq
            -0.025f,   // lpfQ
            6.29f,     // hpfFreq
            -0.042f,   // hpfQ
            0.219f,    // consoleDrive
            0.049f     // outputGain
        },

        // Channel 46
        {
            -0.108f,   // eqTrebleGain
            -3.67f,    // eqTrebleFreq
            0.239f,    // eqBassGain
            -8.24f,    // eqBassFreq
            -7.43f,    // eqBell1Freq
            -0.329f,   // eqBell1Gain
            -0.043f,   // eqBell1Q
            4.91f,     // eqBell2Freq
            0.182f,    // eqBell2Gain
            0.033f,    // eqBell2Q
            -51.4f,    // lpfFreq
            0.056f,    // lpfQ
            -4.53f,    // hpfFreq
            0.058f,    // hpfQ
            -0.152f,   // consoleDrive
            -0.059f    // outputGain
        },

        // Channel 47
        {
            0.257f,    // eqTrebleGain
            8.96f,     // eqTrebleFreq
            -0.121f,   // eqBassGain
            7.28f,     // eqBassFreq
            9.61f,     // eqBell1Freq
            0.274f,    // eqBell1Gain
            0.026f,    // eqBell1Q
            -5.76f,    // eqBell2Freq
            -0.197f,   // eqBell2Gain
            -0.037f,   // eqBell2Q
            85.2f,     // lpfFreq
            0.019f,    // lpfQ
            8.72f,     // hpfFreq
            0.047f,    // hpfQ
            0.189f,    // consoleDrive
            0.066f     // outputGain
        },

        // Channel 48
        {
            -0.165f,   // eqTrebleGain
            -7.29f,    // eqTrebleFreq
            0.108f,    // eqBassGain
            3.47f,     // eqBassFreq
            -4.69f,    // eqBell1Freq
            -0.235f,   // eqBell1Gain
            0.052f,    // eqBell1Q
            8.38f,     // eqBell2Freq
            0.261f,    // eqBell2Gain
            -0.044f,   // eqBell2Q
            -88.3f,    // lpfFreq
            -0.049f,   // lpfQ
            -6.14f,    // hpfFreq
            -0.029f,   // hpfQ
            -0.223f,   // consoleDrive
            -0.081f    // outputGain
        }
    }};

} // namespace ChannelVariations
