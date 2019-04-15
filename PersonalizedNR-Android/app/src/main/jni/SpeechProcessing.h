//
//  SpeechProcessing.h
//
//
//  Created by Abhishek Sehgal on 8/14/17.
//  Modified by Chowdhury, Tahsin on 11/02/17 for integration
//  Copyright © 2017 SIPLab. All rights reserved.
//

#ifndef __Speech_Processing_Pipeline__SpeechProcessing__
#define __Speech_Processing_Pipeline__SpeechProcessing__

#include <stdio.h>
#include <math.h>
#include "Other/FIRFilter.h"
#include "Other/Transforms.h"
#include "Other/SPLBuffer.h"
#include "Settings.h"
#include "CompressionSettingController.h"
#include "rt_nonfinite.h"
#include "wiener_ADAPTIVE_NE.h"
#include "wiener_ADAPTIVE_NE_terminate.h"
#include "wiener_ADAPTIVE_NE_initialize.h"
#include "DynamicRangeFiveBandCompression.h"
#include "DynamicRangeFiveBandCompression_initialize.h"
#include "DynamicRangeFiveBandCompression_terminate.h"
#include "FeatureExtraction/MelSpectrogram.h"
#include "UNC/Equalizer.h"
#include "UNC/Art2FusionClassifier.h"
#include "FeatureExtraction/SubbandFeatures.h"

#define NFILT               40
#define FREQLOW             300
#define FREQHIGH            8000
#define DECIMATION_FACTOR 3
#define EPS 1.0e-7
#define S2F 3.051757812500000e-05f
#define F2S 32768

typedef struct VADNoiseReductionCompression {
    
    int stepSize;
    int decimatedStepSize;
    int decimatedWindowSize;
    int overlap;
    float decimatedFs;
    float* input;
    float* downsampled;
    float* decimated;
    float* noise_reduced;
    float* inputBuffer;
    float* compressed;
    
    float* interpolated;
    float* output;

    FIR* downsampleFilter;
    FIR* interpolationFilter;
    
    // For audio level
    Transform* fft;
    SPLBuffer* spl;

    MelSpectrogram* melSpectrogram;
    SubbandFeatures* sbf;

    //for classifier
    int firstFrame;
    int count;
    int VADClass;
    int ClusterLabel;
    int previousDetectedClass;
    int totalClusters;
    int warmUp;
    const char* className;
    float* AllFeaturesVector;
    float* sumBufferMFSC;
    Equalizer* Equalization;
    //--------- Art2 Noise Classifier --------------
    int NewClusterCreationBufferSize;
    int DecisionSmoothingBufferSize;
    Art2FusionParallel* Art2Fusion;
    //----------------------------------------------
    
} VADNoiseReductionCompression;


VADNoiseReductionCompression* initVAD_NoiseReduction_Compression(Settings* settings, const char *NoiseClassifierFile, const char *UserBandGainsFile);
//****iOS takes short as Input buffer***//
//void doNoiseReduction_Compression_withVAD(VADNoiseReductionCompression *_ptr, short *_in, short* _out, Settings* settings);
//***SuperpoweredSDK(used in Android) handles short to float conversion input buffer***//
void doNoiseReduction_Compression_withVAD(VADNoiseReductionCompression *_ptr, float *_in, float* _out, Settings* settings);
void destroyVAD_NoiseReduction_Compression(VADNoiseReductionCompression** _ptr);
int getClusterLabel (VADNoiseReductionCompression *_ptr);
int getPreviousClusterLabel (VADNoiseReductionCompression *_ptr);
int getTotalClusters(VADNoiseReductionCompression *_ptr);
#endif /* defined(__Speech_Processing_Pipeline__SpeechProcessing__) */

