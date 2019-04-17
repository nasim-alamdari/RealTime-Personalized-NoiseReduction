//
//  AudioSettingsController.swift
//  SPP_Integrated_App
//
//  Created by Akshay Chitale on 6/25/17.
//  Modified by Nasim Alamdari on 07/30/2018
//  Copyright © 2017 UT Dallas. All rights reserved.
//

import Foundation

/// Globally availible audio settings, set in `AppDelegate.swift` on app launch and for use in all view controllers.
var audioController: AudioSettingsController!

/// A controller for starting and stopping the audio, as well as changing the audio settings.
///
/// This class serves as an interface for Swift code to interact with the underlying Objective-C and C audio code.
class AudioSettingsController {
    // Later have private var for IosAudioController, for now is global
    
    
    /// Initializes the `AudioSettingsController` with "Low", "Medium", and "High" gains and default settings.
    init() {
        iosAudio = IosAudioController()
    }
    
    deinit {
        // Deallocate memory
        iosAudio.destructSettings()
    }
 
    
    /// Starts the audio.
    ///
    /// This function is safe to use whether or not the audio is currently stopped.
    func start() {
        if !playAudio {
            iosAudio.start()
            iosAudio.settings.pointee.playAudio = 1
        }
    }
    
    /// Stops the audio.
    ///
    /// This function is safe to use whether or not the audio is currently playing.
    func stop() {
        if playAudio {
            iosAudio.stop()
            iosAudio.settings.pointee.playAudio = 0
        }
    }
    
    
    /// The number of gains an array of gains must have.
//    var numGains: Int {
//        return Int(iosAudio.settings.pointee.numGains)
//    }
    
    /// The current sampling frequency.
    var fs: Int {
        return Int(iosAudio.settings.pointee.fs)
    }
    
    /// The current frame size, a.k.a. the window size.
    var frameSize: Int {
        return Int(iosAudio.settings.pointee.frameSize)
    }
    
    /// The current step size, a.k.a. the overlap size.
    var stepSize: Int {
        return Int(iosAudio.settings.pointee.stepSize)
    }
    
    /// The current microphone status. Is using audio input from the microphone if true, or from a file if false.
    var micStatus: Bool {
        return iosAudio.settings.pointee.micStatus != 0 ? true : false
    }
    
    /// Whether audio is currently playing.
    var playAudio: Bool {
        return iosAudio.settings.pointee.playAudio != 0 ? true : false
    }
    
    var classLabel: Int { // VAD output ( 0 is noise, 1 is speech+noise)
        return Int(iosAudio.settings.pointee.classLabel)
        //return Int(iosAudio.settings.pointee.finalVADLabel) // using VAD class after applying smoothing buffer
    }
    
    
    /// The current output type. Is outputting processed audio if true, or unprocessed audio if false.
    var noiseReductionOutputType: Bool {
        return iosAudio.settings.pointee.noiseReductionOutputType != 0 ? true : false
    }
    
    var compressionOutputType: Bool {
        return iosAudio.settings.pointee.compressionOutputType != 0 ? true : false
    }
    
    
    /// The current amplification of the processed audio output.
    var amplification: Float {
        return Float(iosAudio.settings.pointee.amplification)
    }
    
    /// Whether the gains are being automatically selected. If false, gains are manually selected with `self.currentGains`.
//    var autoGains: Bool {
//        return iosAudio.settings.pointee.autoGains != 0 ? true : false
//    }
    
    /// The audio level, in dB SPL.
    var dbpower: Float {
        return iosAudio.settings.pointee.dbpower
    }
    
    var quietVal: Float {
        return iosAudio.settings.pointee.quiet
    }
    
    var processTime: Float {
        return iosAudio.settings.pointee.processTime
    }
    
    /// The calibration constant for audio level measuring, in dB.
    var calibration: Float {
        return iosAudio.settings.pointee.calibration
    }
    
    /// The interval to wait before updating the audio level, in seconds.
    var dbUpdateInterval: Float {
        return iosAudio.settings.pointee.dbUpdateInterval
    }
    
    var noiseEstimateTime: Float {
        return iosAudio.settings.pointee.noiseEstimateTime
    }
    

  // ** For noise classifier parameters and personalized gains **********
    var saveClassification : Bool {
        return iosAudio.settings.pointee.saveData != 0 ? true : false
    }
    
    var hybridClassification: Bool {
        return iosAudio.settings.pointee.hybridMode != 0 ? true : false
    }
    
    var vigilanceVal1 : Float{
        return iosAudio.settings.pointee.vigilance1
    }
    
    var vigilanceVal2 : Float {
        return iosAudio.settings.pointee.vigilance2
    }
    
    var FeatureAvgSize: Int {
        return Int(iosAudio.settings.pointee.FeatAvgBufferLength)
    }
    
    var NewClassCreationTime: Float {
        return iosAudio.settings.pointee.NewClusterCreationBufferTime
    }
    
    var DecsionClassTime : Float {
        return iosAudio.settings.pointee.DecisionSmoothingBufferTime
    }
    
    
    //var personalizedGains : (Float, Float, Float, Float, Float) {
        //return iosAudio.settings.pointee.userBandGains
    //}
    
    var personalizedGainBand1 : Float {
        return iosAudio.settings.pointee.userGainBand1
    }
    
    var personalizedGainBand2 : Float {
        return iosAudio.settings.pointee.userGainBand2
    }
    
    var personalizedGainBand3 : Float {
        return iosAudio.settings.pointee.userGainBand3
    }
    
    var personalizedGainBand4 : Float {
        return iosAudio.settings.pointee.userGainBand4
    }
    
    var personalizedGainBand5 : Float {
        return iosAudio.settings.pointee.userGainBand5
    }
    
    // Noise classifier resutls
    var DetectedNoiseClass: Int32 {
        return iosAudio.settings.pointee.ClusterLabel
    }
    
    var TotalNoiseClasses : Int32{
        return iosAudio.settings.pointee.totalClusters
    }
    
    var prevDetectNoiseClass: Int32 {
        return iosAudio.settings.pointee.previousDetectedClass
    }
    
    //*****************************************************************
    

    

    /// Updates the sampling frequency.
    ///
    /// - Parameter fs: The new sampling frequency, in Hertz.
    func update(fs: Int) {
        iosAudio.settings.pointee.fs = Int32(fs)
    }
    
    /// Updates the frame size, a.k.a. the window size.
    ///
    /// - Parameter frameSize: The new frame size, in milliseconds.
    func update(frameSize: Float) {
        iosAudio.settings.pointee.frameSize = Int32(frameSize * Float(fs) / 1000.0)
    }
    
    /// Updates the step size, a.k.a. the overlap size.
    ///
    /// - Parameter stepSize: The new step size, in milliseconds.
    func update(stepSize: Float) {
        iosAudio.settings.pointee.stepSize = Int32(stepSize * Float(fs) / 1000.0)
    }
    func update(classLabel: Int) {
        iosAudio.settings.pointee.classLabel = Int32(classLabel)
    }
    /// Updates the microphone status.
    ///
    /// - Parameter micStatus: If true, use microphone audio input. If false, use audio input from file.
    func update(micStatus: Bool) {
        iosAudio.settings.pointee.micStatus = Int32(micStatus ? 1 : 0)
    }
    
    /// Updates the noise reduction output type.
    ///
    /// - Parameter noiseReductionOutputType: If true, play processed audio. If false, play unprocessed audio.
    func update(noiseReductionOutputType: Bool) {
        iosAudio.settings.pointee.noiseReductionOutputType = Int32(noiseReductionOutputType ? 1 : 0)
    }
    
    func update(compressionOutputType: Bool) {
        iosAudio.settings.pointee.compressionOutputType = Int32(compressionOutputType ? 1 : 0)
    }
    
    func update(doSaveFile: Bool) {
        iosAudio.settings.pointee.doSaveFile = Int32(doSaveFile ? 1 : 0)
    }
    
    // Update quiet value
    func update(quietVal: Float){
        iosAudio.settings.pointee.quiet = Float(quietVal)
    }
    
    
    /// Updates the calibration constant for audio level calculation
    ///
    /// - Parameter calibration: The new calibration constant for audio level measuring, in dB.
    func update(calibration: Float) {
        iosAudio.settings.pointee.calibration = calibration
    }
    
    /// Updates the interval to wait before updating the audio level.
    ///
    /// - Parameter dbUpdateInterval: The new interval to wait, in seconds.
    func update(dbUpdateInterval: Float) {
        iosAudio.settings.pointee.dbUpdateInterval = dbUpdateInterval
    }
    
    func update(noiseEstimateTime: Float) {
        iosAudio.settings.pointee.noiseEstimateTime = noiseEstimateTime
        iosAudio.settings.pointee.noiseEstimationFrame = noiseEstimateTime*Float(fs)/Float(stepSize)
    }
    
    // ** Update For noise classifier parameters and personalized gains **********
    func update(saveClassification : Bool) {
        iosAudio.settings.pointee.saveData  = Int32(saveClassification ? 1 : 0)
    }
    
    func update(hybridClassification: Bool) {
        iosAudio.settings.pointee.hybridMode = Int32(hybridClassification ? 1 : 0)
    }
    
    func update(vigilanceVal1 : Float) {
        iosAudio.settings.pointee.vigilance1 = Float(vigilanceVal1)
    }
    
    func update(vigilanceVal2 : Float) {
        iosAudio.settings.pointee.vigilance2 = Float(vigilanceVal2)
    }
    
    func update(FeatureAvgSize: Int) {
        iosAudio.settings.pointee.FeatAvgBufferLength = Int32(FeatureAvgSize)
    }
    
    func update(NewClassCreationTime: Float) {
        iosAudio.settings.pointee.NewClusterCreationBufferTime = Float(NewClassCreationTime)
    }
    
    func update(DecsionClassTime : Float){
        iosAudio.settings.pointee.DecisionSmoothingBufferTime = Float(DecsionClassTime)
    }
    
    //func update(personalizedGains : (Float, Float, Float, Float, Float) ) {
        //iosAudio.settings.pointee.userBandGains = personalizedGains
    //}
    
    func update(personalizedGainBand1 : Float ) {
        iosAudio.settings.pointee.userGainBand1 = Float(personalizedGainBand1)
    }
    
    func update(personalizedGainBand2 : Float ) {
        iosAudio.settings.pointee.userGainBand2 = Float(personalizedGainBand2)
    }
    
    func update(personalizedGainBand3 : Float ) {
        iosAudio.settings.pointee.userGainBand3 = Float(personalizedGainBand3)
    }
    
    func update(personalizedGainBand4 : Float ) {
        iosAudio.settings.pointee.userGainBand4 = Float(personalizedGainBand4)
    }
    
    func update(personalizedGainBand5 : Float ) {
        iosAudio.settings.pointee.userGainBand5 = Float(personalizedGainBand5)
    }
    
    
    //*****************************************************************
    

    ///
    /// - Parameter alpha: The new weight of `self.lowDef` in the boundary calculation.
    func update(alpha: Float) {
        iosAudio.settings.pointee.alpha = alpha
    }

    /// - Parameter beta: The new weight of `self.mediumDef` in the boundary calculation.
    func update(beta: Float) {
        iosAudio.settings.pointee.beta = beta
    }

    /// Updates the amplification.
    ///
    /// - Parameter amplification: The new amplification value.
    func update(amplification: Float) {
        iosAudio.settings.pointee.amplification = amplification
    }
    
}
