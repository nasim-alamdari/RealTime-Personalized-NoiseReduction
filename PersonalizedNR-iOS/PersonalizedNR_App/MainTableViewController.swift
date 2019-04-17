//
//  MainTableViewController.swift
//  SPP_Integrated_App
//
//  Created by Akshay Chitale on 6/12/17.
//  Modified by Nasim Alamdari on 07/30/2018
//  Copyright © 2017 UT Dallas. All rights reserved.
//

import UIKit
import Foundation
//var mainTableViewController: MainTableViewController!
/// The view controller for the root view of the app.
class MainTableViewController: UITableViewController {
    
    // Outlets to UI components
    @IBOutlet weak var switchSaveOutput: UISwitch!
    @IBOutlet weak var switchOutputType: UISwitch!
    @IBOutlet weak var switchCompression: UISwitch!
    @IBOutlet weak var labelAmplification: UILabel!
    @IBOutlet weak var sliderAmplification: UISlider!
    @IBOutlet weak var LabelQiuetAjustment: UILabel!
    @IBOutlet weak var sliderQuietAdjust: UISlider!
    @IBOutlet weak var labelStartStop: UILabel!
    @IBOutlet weak var noise_label: UILabel!
    @IBOutlet weak var speech_label: UILabel!
    @IBOutlet weak var quiet_label: UILabel!
    @IBOutlet weak var UTDlogoLabName: UILabel!
    @IBOutlet weak var utdLogo: UIImageView!
    @IBOutlet weak var LabelFrameProcTime: UILabel!
    @IBOutlet weak var labelDetectedNoiseClass: UILabel!
    @IBOutlet weak var labelTotalDetectedNoiseClass: UILabel!
    
    var refreshVad: Timer!
    var refreshFrameTime: Timer!
    // local vars
    /// The green color for the start button text.
    var ratingText: String!
    let startColor: UIColor = UIColor(red: 56.0/256, green: 214.0/256, blue: 116.0/256, alpha: 1.0) // Same as green on switch
    var path: URL = MainTableViewController.documentsDirectory.appendingPathComponent("Ratings.txt")
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let bounds = self.navigationController!.navigationBar.bounds
        let myView: UIView = UIView()
        let title: UILabel = UILabel(frame: CGRect(x:-135, y:-10, width:300, height:30))

        title.text = "Personalized Noise Reduction"//"Signal and Image Processing Lab"

        let image: UIImage = UIImage(named: "utd_logo.png")!
       //let myImageView: UIImageView = UIImageView(image: image)
       //myImageView.frame = CGRect(x:-170, y:-10, width:30, height:30)
       // myImageView.align
       // let height: CGFloat = 50 //whatever height you want
        
        //self.navigationController!.navigationBar.frame = CGRect(x: 0, y: 0, width: bounds.width, height: height)
        myView.addSubview(title)
        //myView.addSubview(myImageView)
        
        UTDlogoLabName.text = "Signal and Image Processing Lab"
        utdLogo.image =  image;
        
        self.navigationItem.titleView = myView

      //  self.navigationItem.titleView.pos
        
        ratingText = ""
        noise_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0 )
        speech_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0 )
        quiet_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0 )

        tableView.beginUpdates()
        tableView.endUpdates()
        
        // Output Type switch
        switchOutputType.isOn = audioController.noiseReductionOutputType
        switchOutputType.isEnabled = audioController.playAudio
        switchCompression.isOn = audioController.compressionOutputType;
        switchCompression.isEnabled = audioController.playAudio
        
        // Amplification slider
        sliderAmplification.value = ampMap(audioController.amplification, sliderToAmp: false)
        sliderAmplification.isEnabled = false
        labelAmplification.text = "Amplification: \(String(format: "%0.2f", audioController.amplification))x"
        
        // Quiet Adjustment slider
        sliderQuietAdjust.value = QuietMap(audioController.quietVal, sliderToQuiet: false)
        sliderQuietAdjust.isEnabled = true
        LabelQiuetAjustment.text = "Quiet Adjustment: \(String(format: "%0.0f", audioController.quietVal)) dB"
        
        
        refreshProcTime()
        // Timer to refresh SPL
        refreshFrameTime = Timer.scheduledTimer(timeInterval: TimeInterval(audioController.dbUpdateInterval),
                                       target: self,
                                       selector: #selector(self.refreshProcTime),
                                       userInfo: nil,
                                       repeats: true)
        // Update VAD output
        refreshVad = Timer.scheduledTimer(timeInterval: TimeInterval(audioController.dbUpdateInterval),
                                          target: self,
                                          selector: #selector(self.refreshVADOut),
                                          userInfo: nil,
                                          repeats: true)
        
        // Set start/stop colors
        startStop(start: audioController.playAudio, setMic: true) // Show stopped or started
        
//                    DispatchQueue.main.async (execute: {
//                        self.changeLabel(vadOutput: audioController.classLabel)
//                    })
        
//                    DispatchQueue.global(qos: .userInitiated).async {
//                        //let image = self.loadOrGenerateAnImage()
//                        // Bounce back to the main thread to update the UI
//                        DispatchQueue.main.async {
//                            self.changeLabel(vadOutput: audioController.classLabel)
//                        }
//                    }
        
      
        let longPressToExitKeyboard: UILongPressGestureRecognizer =
            UILongPressGestureRecognizer(target: self,
                                   action: #selector(self.dismissKeyboard))
        view.addGestureRecognizer(longPressToExitKeyboard)
    }
    
    func dismissKeyboard() {
        view.endEditing(true) // End editing on tap
    }
    
    func refreshVADOut() {
        self.changeLabel(vadOutput: audioController.classLabel)
    }
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
    }
    
    /// Refreshes the frame processing time (ms).
    func refreshProcTime() {
        LabelFrameProcTime.text = "\(String(format:"%.2f", audioController.processTime)) ms"
        labelDetectedNoiseClass.text = "\(String(format:"%d", audioController.DetectedNoiseClass))"
        labelTotalDetectedNoiseClass.text = "\(String(format:"%d", audioController.TotalNoiseClasses))"
    }
    
    
    deinit {
        // Remove the notification
        NotificationCenter.default.removeObserver(self)
    }
    
    static var documentsDirectory: URL {
        get {
            let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
            let documentsDirectory = paths[0]
            return documentsDirectory
        }
    }
    
    
    func showErrorAlert(_ message: String) {
        let alert: UIAlertController = UIAlertController(title: "Save File Error",
                                                         message: message,
                                                         preferredStyle: .alert)
        let ok = UIAlertAction(title: "OK", style: .default, handler: {
            UIAlertAction in
        })
        alert.addAction(ok)
        self.present(alert, animated: true, completion: nil)
    }
    
    @IBAction func saveOutputinFile(_ sender: UISwitch) {
        audioController.update(doSaveFile: sender.isOn)
    }
    
    // Audio Output switch
    @IBAction func switchOutputTypeValueChanged(_ sender: UISwitch) {
        audioController.update(noiseReductionOutputType: sender.isOn)
        if sender.isOn {
            labelAmplification.text = "Amplification: \(String(format: "%0.2f", audioController.amplification))x"
            sliderAmplification.value = ampMap(audioController.amplification, sliderToAmp: false)
        }
    }
    
    @IBAction func compressionOutputTypeValueChanded(_ sender: UISwitch) {
        audioController.update(compressionOutputType: sender.isOn)
        if sender.isOn {
            labelAmplification.text = "Amplification: \(String(format: "%0.2f", audioController.amplification))x"
            sliderAmplification.value = ampMap(audioController.amplification, sliderToAmp: false)
        }
    }
    // Amplification slider
    @IBAction func sliderAmplificationValueChanged(_ sender: UISlider) {
        audioController.update(amplification: ampMap(sender.value, sliderToAmp: true))
        labelAmplification.text = "Amplification: \(String(format: "%0.2f", audioController.amplification))x"
    }
    
    //Quiet Adjustment slider update
    let step: Float = 1
    @IBAction func sliderQuietAdjustmentChanged(_ sender: UISlider) {
        let roundedValue = round(sender.value / step) * step
        sender.value = roundedValue
        audioController.update(quietVal: QuietMap(sender.value, sliderToQuiet: true))
        LabelQiuetAjustment.text = "Quiet Adjustment: \(String(format: "%0.0f", audioController.quietVal)) dB"
    }
    
    /// Starts or stops the audio.
    ///
    /// - Parameters:
    ///   - start: Starts the audio if true, or stops the audio if false.
    ///   - setMic: Enables the microphone input on start if true.
    func startStop(start: Bool, setMic: Bool) {
        // setMic true for playing back input, false for playing audio from file
        if start {
            labelStartStop.text = "STOP"
            labelStartStop.textColor = UIColor.red
            audioController.update(micStatus: setMic)
            audioController.start()
            // Set button enables
            switchOutputType.isEnabled = true
            switchCompression.isEnabled = true
            switchSaveOutput.isEnabled = false
            sliderAmplification.isEnabled = true
            sliderQuietAdjust.isEnabled = false
            noise_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
            speech_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
            quiet_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0 )
        }
        else {
            labelStartStop.text = "LIVE AUDIO"
            labelStartStop.textColor = startColor
            audioController.stop()
            audioController.update(micStatus: false)
            // Set button enables
            switchOutputType.isEnabled = false
            switchCompression.isEnabled = false
            switchSaveOutput.isEnabled = true
            sliderAmplification.isEnabled = false
            sliderQuietAdjust.isEnabled = true
            audioController.update(classLabel: -1)
        }
    }
    
    func changeLabel(vadOutput: Int)
    {
        switch (vadOutput) {
            case 0: //Quiet class
                quiet_label.backgroundColor = UIColor( red:0, green:0.1, blue:0.6, alpha:0.25)
                noise_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                speech_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                break;
            case 1: // noise class
                quiet_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                noise_label.backgroundColor = UIColor( red:0.6, green:0, blue:0.3, alpha:0.25)
                speech_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                break;
            case 2: // speech+noise class
                quiet_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                noise_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                speech_label.backgroundColor = UIColor( red:0, green:0.6, blue:0.3, alpha:0.25)
            break;
            
            default:
                quiet_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                noise_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
                speech_label.backgroundColor = UIColor( red:0, green:0, blue:0, alpha:0)
            break;
        }
    }
    
    /// Shows an error alert if an error occurs when importing files.
    ///
    /// - Parameter message: The error message to show in the alert.
    func showImportErrorAlert(_ message: String) {
        let alert: UIAlertController = UIAlertController(title: "Import File Error",
                                                         message: message,
                                                         preferredStyle: .alert)
        let ok = UIAlertAction(title: "OK", style: .default, handler: {
            UIAlertAction in
        })
        alert.addAction(ok)
        self.present(alert, animated: true, completion: nil)
    }
        
    /// A function to map slider value to amplification value.
    ///
    /// - Parameters:
    ///   - value: The value to map.
    ///   - sliderToAmp: Map a slider position to an amplification if true, or map an amplification value to a slider position if false.
    /// - Returns: The mapped value.
    func ampMap(_ value: Float, sliderToAmp: Bool) -> Float {
        if sliderToAmp {
            return value * value
        }
        else {
            return sqrtf(value)
        }
    }
    
    //Mapping quiet adjustment slider
    func QuietMap(_ value: Float, sliderToQuiet: Bool) -> Float {
        if sliderToQuiet {
            return step * value
        }
        else {
            return Float(value/step)
        }
    }
    
    // Hide and show rows
    override func tableView(_ tableView: UITableView, estimatedHeightForRowAt indexPath: IndexPath) -> CGFloat {
        return UITableViewAutomaticDimension
    }
    
    // Selecting row which functions as a button
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if indexPath.section == 4 && indexPath.row == 1 {
            startStop(start: !audioController.playAudio, setMic: true) // If it's playing, stop it; if not, start it
            tableView.deselectRow(at: indexPath, animated: true)
        }
    }
    
}
