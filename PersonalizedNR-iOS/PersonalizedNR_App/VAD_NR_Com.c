//
//  VAD_NR_Com.c
//  Voice Activity Detection with NoiseReduction and compression
//
//  Created by Abhishek Sehgal on 8/14/17.
//  Modified by Nasim Alamdari on 07/30/2018
//  Copyright © 2017 SIPLab. All rights reserved.
//


#include <sys/time.h>
#include "VAD_NR_Com.h"

#define SubbandFeatureNum 8
#define AllFeaturesSize 48
struct timeval t1, t2;
double elapsedTime;
float dataIn[20] = {5, -5, 5, 100,
    5, -10, 5, 100,
    5, -20, 2, 50,
    4, -25, 2, 50,
    4, -35, 2, 100};

VADNoiseReductionCompression* initVAD_NoiseReduction_Compression(Settings* settings, const char *NoiseClassifierFile, const char *UserBandGainsFile) {

    VADNoiseReductionCompression* inParam = (VADNoiseReductionCompression*)malloc(sizeof(VADNoiseReductionCompression));
    
    inParam->stepSize           = settings->stepSize;
    inParam->decimatedStepSize  = settings->stepSize/DECIMATION_FACTOR;
    inParam->samplingFrequency  = settings->fs/DECIMATION_FACTOR;
    
    inParam->input          = (float*)calloc(settings->stepSize, sizeof(float));
    inParam->downsampled    = (float*)calloc(settings->stepSize, sizeof(float));
    inParam->decimated      = (float*)calloc(inParam->decimatedStepSize, sizeof(float));
    inParam->processed      = (float*)calloc(inParam->decimatedStepSize, sizeof(float));
    inParam->frame          = (float*)calloc(inParam->decimatedStepSize*2, sizeof(float));
    
    /*for compression*/
    inParam->compressed      = (float*)calloc(inParam->decimatedStepSize, sizeof(float));
    /**/
    
    inParam->interpolated   = (float*)calloc(settings->stepSize, sizeof(float));
    inParam->output         = (float*)calloc(settings->stepSize, sizeof(float));
    
    /*for VAD initialization*/
    inParam->fft            = newTransform(2*inParam->decimatedStepSize,
                                           (int)(inParam->samplingFrequency/inParam->decimatedStepSize));
    inParam->melSpectrogram = initMelSpectrogram(NFILT, FREQLOW, FREQHIGH,
                                                 2*inParam->decimatedStepSize,
                                                 inParam->samplingFrequency,
                                                 inParam->fft->points);
    
     /**/
    
    inParam->downsampleFilter       = initFIR(settings->stepSize);
    inParam->interpolationFilter    = initFIR(settings->stepSize);
    

    
    // For audio level
    inParam->spl = newSPLBuffer(settings->dbUpdateInterval * settings->fs / settings->stepSize, SPLBUFFER_MODE_FULL);
    
    inParam->sbf = initSubbandFeatures(inParam->fft->points,inParam->decimatedStepSize, 80); //decision buffer length
    
    
    /*for noise classifier and equalizer*/
    inParam->firstFrame          = 1; //only for SUBBAND FEATURES
    inParam->count               = 0;
    inParam->VADClass            = 0;
    inParam->ClusterLabel        = 1;
    inParam->previousDetectedClass = 1;
    inParam->totalClusters       = 1;
    inParam->warmUp              = 80; //inParam->decisionBufferLength; //decisionBufferLength;
    inParam->className           = "className";
    inParam->sumBufferMFSC       = (float*)calloc(40, sizeof(float));
    inParam->AllFeaturesVector   = (float*)calloc(AllFeaturesSize ,sizeof(float));// 40 is size of filters in Log Mel
    
    //===== For noise classifier ====================================
    inParam->NewClusterCreationBufferSize = (int)floor((settings->NewClusterCreationBufferTime * 1000)/ (12.5 * settings->FeatAvgBufferLength) ); //convert from seconds to integer size;
    inParam->DecisionSmoothingBufferSize  = (int)floor((settings->DecisionSmoothingBufferTime * 1000)/ (12.5 * settings->FeatAvgBufferLength) ); //convert from seconds to integer size;
    inParam->Art2Fusion = initArt2ParallelFusion(settings->saveData,
                                                 settings->hybridMode,
                                                 settings->vigilance1,
                                                 settings->vigilance2,
                                                 NoiseClassifierFile,
                                                 8, // number of subband features
                                                 40, // number of MFSC features
                                                 inParam->NewClusterCreationBufferSize,
                                                 inParam->DecisionSmoothingBufferSize,
                                                 10); // maximum number of noise classes creation
    // Personalized gains initialization
    inParam->Equalization = initEqualizer(settings->saveData,
                                          settings->hybridMode,
                                          UserBandGainsFile,
                                          inParam->Art2Fusion->MaxClusterNum,
                                          inParam->Art2Fusion->TotalDetectedClass);
    
    
    wiener_ADAPTIVE_NE_initialize();
    DynamicRangeFiveBandCompression_initialize();
    return inParam;
}


void doNoiseReduction_Compression_withVAD(VADNoiseReductionCompression *_ptr,
                                          short *_in,
                                          short* _out,
                                          Settings* settings){

    
    VADNoiseReductionCompression *inParam = _ptr;
    
    int i,j;
    
    
    for (i = 0; i < inParam->stepSize; i++) {
        inParam->input[i] = _in[i] * S2F;
    }
    
    //Downsample the Audio
    processFIRFilter(inParam->downsampleFilter, inParam->input, inParam->downsampled);
    
    //Decimate the Audio
    for(i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+= 3){
        inParam->decimated[i] = inParam->downsampled[j];
        
        inParam->frame[i] = inParam->frame[i+inParam->decimatedStepSize];
        inParam->frame[i+inParam->decimatedStepSize] = inParam->decimated[i];
    }
    
    ForwardFFT(inParam->fft, inParam->frame, settings->calibration);
    updateImage(inParam->melSpectrogram, inParam->fft->power);
    settings->dbpower = inParam->fft->dbpower;
    /***---------------Noise Classifier and Equalizer----------------------------***/
    
    //inParam->VADClass = ((settings->dbpower < settings->quiet) ? 0 : settings->classLabel + 1);
    if(settings->classLabel == 1) { //settings->finalVADLabel == 1  // settings->classLabel == 1 means only noise frames or quiet frames
        computeSubbandFeatures(inParam->sbf, inParam->fft->power, inParam->firstFrame);
        inParam->firstFrame = 0;
        melCalculate(inParam->fft->power, inParam->melSpectrogram->nFFT, inParam->melSpectrogram->filtBank, inParam->melSpectrogram->nFilt, inParam->melSpectrogram->melPower);
        for ( i = 0; i < 40; i++)
        {
            inParam->sumBufferMFSC[i] = inParam->sumBufferMFSC[i] + inParam->melSpectrogram->melPower[i];
        }
        
        inParam->count++;
        
        //When the count is more than the decision buffer length
        //start warmUp condition
        if (inParam->count > inParam->warmUp -1 ) {
            
            
            for (i= 0 ; i < SubbandFeatureNum; i++) {
                inParam->AllFeaturesVector[i] = inParam->sbf->subbandFeatureList[i];
            }
            int index = 0;
            for (i= SubbandFeatureNum; i < AllFeaturesSize; i++) {
                inParam->AllFeaturesVector[i] = inParam->sumBufferMFSC[i]/inParam->count;
                inParam->sumBufferMFSC[index] = 0;
                index++;
            }
            inParam->previousDetectedClass = inParam->ClusterLabel;
            settings->previousDetectedClass = inParam->previousDetectedClass;
            computeArt2ParallelFusion(inParam->Art2Fusion,inParam->AllFeaturesVector, inParam->className);
            
            inParam->ClusterLabel  = inParam->Art2Fusion->detectedClass;
            inParam->totalClusters = inParam->Art2Fusion->TotalDetectedClass;
            
            settings->ClusterLabel = inParam->ClusterLabel;
            settings->totalClusters = inParam->totalClusters;
            //inParam-> frameNumber = inParam->Clustering_ptr->frameNumber;
            inParam->count = 0;
        }
    }
    
    //getSmoothBandGains(inParam->Equalization, settings->userBandGains, 1, 1, 1);
    settings->userBandGains[0] = settings->userGainBand1;
    settings->userBandGains[1] = settings->userGainBand2;
    settings->userBandGains[2] = settings->userGainBand3;
    settings->userBandGains[3] = settings->userGainBand4;
    settings->userBandGains[4] = settings->userGainBand5;
    getSmoothBandGains(inParam->Equalization, settings->userBandGains, settings->ClusterLabel, settings->previousDetectedClass, settings->totalClusters);
    
    /***---------------Noise Classifier and Equalizer ENDS----------------------------***/
    
    /*
    int i,j;
    
    // Convert from Short to Float
    for (i = 0; i < inParam->stepSize; i++) {
        inParam->input[i] = _in[i] * S2F;
    }
    
    // Downsample the audio
    processFIRFilter(inParam->downsampleFilter, inParam->input, inParam->downsampled);
    
    // Decimate the audio
    for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+= 3) {
        inParam->decimated[i] = inParam->downsampled[j];
    }
    //VAD computation
    compute(inParam->memoryPointer, inParam->decimated, 52.0f, settings->calibration);

    // SAVE Audio level to Settings
    Variables* vars = (Variables*)inParam->memoryPointer;
    inParam->spl->buffer(inParam->spl, vars->fft->dbpower, settings);
    
    */
    
    //Adaptive Noise Reduction
    inParam->VADClass = settings->classLabel;
    if (settings->classLabel == 0 ){inParam->VADClass = 1;} // VADClass is either 1 for noise or Quiet, and 1 for speech+noise
    wiener_ADAPTIVE_NE(inParam->decimated,16000, inParam->VADClass, settings->noiseEstimationFrame, inParam->processed, inParam->Equalization->smoothBandGains);

    //when noise reduction is on only
    if (settings->noiseReductionOutputType && !settings->compressionOutputType) {
        
            //Adaptive Noise Reduction
            //wiener_ADAPTIVE_NE(inParam->decimated, -1, 16000, getClass(inParam->memoryPointer), settings->noiseEstimationFrame, inParam->processed);
            //Interpolate the Audio
            for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+=3) {
                inParam->interpolated[j] = inParam->processed[i];
            }
            // Low-Pass filter the Interpolated Audio
            processFIRFilter(inParam->interpolationFilter, inParam->interpolated, inParam->output);
            // Convert to Short from Float
            for (i = 0; i < inParam->stepSize; i++) {
                _out[i] = (short)(settings->amplification*2*F2S * inParam->output[i]);
            }
    }
    //when compression is on only
    else if(settings->compressionOutputType && !settings->noiseReductionOutputType){
        // Compression
        DynamicRangeFiveBandCompression(inParam->decimated, 16000, dataIn, 48, 0, inParam->compressed);
        //Interpolate the Audio
        for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+=3) {
            inParam->interpolated[j] = inParam->compressed[i];
        }
        // Low-Pass filter the Interpolated Audio
        processFIRFilter(inParam->interpolationFilter, inParam->interpolated, inParam->output);
        // Convert to Short from Float
        for (i = 0; i < inParam->stepSize; i++) {
            _out[i] = (short)(settings->amplification*2*F2S * inParam->output[i]);
        }
    }
    // when both noise reduction and compression is on
    else if(settings->noiseReductionOutputType && settings->compressionOutputType){
        //Adaptive Noise Reduction
        //wiener_ADAPTIVE_NE(inParam->decimated, -1, 16000, getClass(inParam->memoryPointer), settings->noiseEstimationFrame, inParam->processed);
        // Compression
        DynamicRangeFiveBandCompression(inParam->processed, 16000, dataIn, 48, 0, inParam->compressed);
        //Interpolate the Audio
        for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+=3) {
            inParam->interpolated[j] = inParam->compressed[i];
        }
        // Low-Pass filter the Interpolated Audio
        processFIRFilter(inParam->interpolationFilter, inParam->interpolated, inParam->output);
        // Convert to Short from Float
        for (i = 0; i < inParam->stepSize; i++) {
            _out[i] = (short)(settings->amplification*2*F2S * inParam->output[i]);
        }
    }
    else {
        memcpy(_out, _in, inParam->stepSize * sizeof(short));
    }
}

void destroyVAD_NoiseReduction_Compression(VADNoiseReductionCompression** _ptr) {
    if (*_ptr != NULL) {
        
        wiener_ADAPTIVE_NE_initialize();
        DynamicRangeFiveBandCompression_terminate();
        
        if ((*_ptr)->input != NULL){
            free((*_ptr)->input);
            (*_ptr)->input = NULL;
        }
        if ((*_ptr)->downsampled != NULL){
            free((*_ptr)->downsampled);
            (*_ptr)->downsampled = NULL;
        }
        if ((*_ptr)->decimated != NULL){
            free((*_ptr)->decimated);
            (*_ptr)->decimated = NULL;
        }
        if ((*_ptr)->processed != NULL){
            free((*_ptr)->processed);
            (*_ptr)->processed = NULL;
        }
        
        /*for compression*/
        if ((*_ptr)->compressed != NULL){
            free((*_ptr)->compressed);
            (*_ptr)->compressed = NULL;
        }
        /**/
        
        if ((*_ptr)->interpolated != NULL){
            free((*_ptr)->interpolated);
            (*_ptr)->interpolated = NULL;
        }
        if ((*_ptr)->output != NULL){
            free((*_ptr)->output);
            (*_ptr)->output = NULL;
        }
        
        /*for VAD Parameters*/
        if ((*_ptr)->memoryPointer != NULL){
            free((*_ptr)->memoryPointer);
            (*_ptr)->memoryPointer = NULL;
        }
        /**/
        
        if ((*_ptr)->AllFeaturesVector != NULL){
            free((*_ptr)->AllFeaturesVector);
            (*_ptr)->AllFeaturesVector = NULL;
        }
        if ((*_ptr)->sumBufferMFSC != NULL){
            free((*_ptr)->sumBufferMFSC);
            (*_ptr)->sumBufferMFSC = NULL;
        }
        
        if ((*_ptr)->spl != NULL){
            free((*_ptr)->spl);
            (*_ptr)->spl = NULL;
        }
        
        destroyFIR(&(*_ptr)->downsampleFilter);
        destroyFIR(&(*_ptr)->interpolationFilter);
        destroyTransform(&(*_ptr)->fft);
        //destroySPLBuffer(&(*_ptr)->spl);
        destroySubbandFeatures(&(*_ptr)->sbf);
        destroyArt2ParallelFusion(&(*_ptr)->Art2Fusion);
        destroyEqualizer(&(*_ptr)->Equalization);
        free(*_ptr);
        *_ptr = NULL;
    }
}

void getMelImage(VADNoiseReductionCompression* memoryPointer, float** melImage){
    VADNoiseReductionCompression* inParam = (VADNoiseReductionCompression*) memoryPointer;
    for (size_t i = 0; i < NFILT; i++) {
        for (size_t j = 0; j < NFILT; j++) {
            melImage[i][j] = inParam->melSpectrogram->melSpectrogramImage[i][j];
        }
    }
}


/*int getClusterLabel (VADNoiseReductionCompression *_ptr){
    VADNoiseReductionCompression *inParam = _ptr;
    
    if(inParam != NULL) {
        return inParam->ClusterLabel;
    }
    else {
        return 0;
    }
    
}

int getPreviousClusterLabel (VADNoiseReductionCompression *_ptr){
    VADNoiseReductionCompression *inParam = _ptr;
    if(inParam != NULL) {
        return inParam->previousDetectedClass;
    }
    else {
        return 0;
    }
}

int getTotalClusters(VADNoiseReductionCompression *_ptr){
    VADNoiseReductionCompression *inParam = _ptr;
    if(inParam != NULL) {
        return inParam->totalClusters;
    }
    else {
        return 0;
    }
}*/

