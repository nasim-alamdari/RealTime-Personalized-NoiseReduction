//
//  Settings.c
//  SPP_Integrated_App
//
//  Created by Akshay Chitale on 6/25/17.
//  Copyright © 2017 UT Dallas. All rights reserved.
//

#import "Settings.h"

Settings* newSettings() {
    
    Settings* newSettings = (Settings*)malloc(sizeof(Settings));
    
    // Set defaults
    newSettings->fs =  48000;
    newSettings->frameSize = 1200; //64;
    newSettings->stepSize = 600; //600;
    newSettings->doSaveFile = 0;
    newSettings->micStatus = 1;
    newSettings->playAudio = 0;
    newSettings->classLabel = -1;
    
    newSettings->noiseReductionOutputType = 0;
    newSettings->compressionOutputType = 0;
//    newSettings->fileName = "TestSignal";
//
//    // Points to arrays from AudioSettingsController.swift, so no calloc/free needed here
//    newSettings->lowDef = 55.0;
//    newSettings->mediumDef = 65.0;
//    newSettings->highDef = 75.0;
//    newSettings->lowGains = NULL;
//    newSettings->mediumGains = NULL;
//    newSettings->highGains = NULL;
//    newSettings->gains = NULL;
//    newSettings->numGains = 5;
    newSettings->amplification = 1.0;
    
    newSettings->dbpower = 0;
    newSettings->quiet = 52.0f;//For iOS
    //newSettings->quiet = 60.0f; // For Android
    newSettings->calibration = -93.9794;
    newSettings->autoGains = 1;
    newSettings->dbUpdateInterval = 0.5;
    newSettings->noiseEstimateTime = 0.4;
//    newSettings->currentGains = 1;
    newSettings->alpha = 0.5;
    newSettings->beta = 0.5;
    newSettings->processTime = 0.0f;
    newSettings->noiseEstimationFrame = newSettings->noiseEstimateTime*(float)(newSettings->fs)/(float)(newSettings->stepSize);

    
    //For noise classifier
    newSettings->saveData = 0;
    newSettings->hybridMode = 0;
    newSettings->vigilance1 = 0.02;
    newSettings->vigilance2 = 0.9;
    newSettings->FeatAvgBufferLength = 80;
    newSettings->NewClusterCreationBufferTime = 3.0f;
    newSettings->DecisionSmoothingBufferTime = 3.0f;
    
    newSettings->userGainBand1 = 1.0f;
    newSettings->userGainBand2 = 1.0f;
    newSettings->userGainBand3 = 1.0f;
    newSettings->userGainBand4 = 1.0f;
    newSettings->userGainBand5 = 1.0f;
    
    newSettings->userBandGains[0] = newSettings->userGainBand1;
    newSettings->userBandGains[1] = newSettings->userGainBand2;
    newSettings->userBandGains[2] = newSettings->userGainBand3;
    newSettings->userBandGains[3] = newSettings->userGainBand4;
    newSettings->userBandGains[4] = newSettings->userGainBand5;
    //int i;
    //for(i =0; i < 5; i++){
        //newSettings->userBandGains[i] = 1.0f;
    //}
    
    //noise classifier resutls
    newSettings->ClusterLabel = 0;
    newSettings->previousDetectedClass = 0;
    newSettings->totalClusters = 0;
    
    // for Smoothing VAD output
    newSettings->melImgFormationSteps = 13;
    newSettings->VADBufferLength = 160; //floor((2*1000)/12.5); // if overlapp window size is 12.5 ms with FrameStepForMel= 13 waiting for 2 ms , then VADbufferSize is 160
    newSettings->VADSmoothBuffer = 0;
    newSettings->finalVADLabel = -1;
    
    return newSettings;
}

void destroySettings(Settings* settings) {
    if(settings != NULL){
        free(settings);
        settings = NULL;
    }
}
