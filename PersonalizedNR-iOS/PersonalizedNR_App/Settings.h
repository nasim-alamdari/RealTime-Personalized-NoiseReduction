//
//  Settings.h
//  SPP_Integrated_App
//
//  Created by Akshay Chitale on 6/25/17.
//  Copyright © 2017 UT Dallas. All rights reserved.
//

#import <stdlib.h>

#ifndef Settings_h
#define Settings_h

typedef struct Settings {
    // Core values
    int fs;
    int frameSize;
    int stepSize;
    
    // Audio status flags
    int micStatus;
    int playAudio;
    
    // To modify output audio type
    int noiseReductionOutputType;
    int compressionOutputType;
    int doSaveFile;
    int classLabel;
    float amplification;
    
    // For audio level based gains switching
    float dbpower;
    float quiet;
    float calibration;
    int autoGains;
    float dbUpdateInterval; // How long to wait (in seconds) before updating dbpower
    float noiseEstimateTime; //Noise estimation time (in seconds)

    float alpha;            // Between 0 and 1, only used if SPLBuffer mode is SPLBUFFER_MODE_RANGE
    float beta;             // Between 0 and 1, only used if SPLBuffer mode is SPLBUFFER_MODE_RANGE
    float processTime;
    float noiseEstimationFrame;
    

    //For noise classifier
    int saveData;
    int hybridMode;
    float vigilance1;
    float vigilance2;
    int FeatAvgBufferLength;
    float NewClusterCreationBufferTime;
    float DecisionSmoothingBufferTime;
    float userBandGains[5];
    float userGainBand1;
    float userGainBand2;
    float userGainBand3;
    float userGainBand4;
    float userGainBand5;
    
    //noise classifier resutls
    int ClusterLabel;
    int previousDetectedClass;
    int totalClusters;
    
    // for Smoothing VAD output
    int melImgFormationSteps;
    int VADBufferLength;
    int VADSmoothBuffer;
    int finalVADLabel;
    
} Settings;

Settings* newSettings(void);
void destroySettings(Settings* settings);

#endif /* Settings_h */
