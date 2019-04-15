package com.superpowered.IntegratedApp;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

public class NoiseClassificationSettings  extends AppCompatActivity {
    SeekBar firstBand, secondBand, thirdBand, fourthBand, fifthBand;
    TextView firstBandLabel, secondBandLabel, thirdBandLabel, fourthBandLabel, fifthBandLabel;//, statusView
    EditText vigilanceValue1, vigilanceValue2, FeatAvgBuffVal,NewClassCreationBuffVal,DecSmoothingBuffVal;
    Switch saveData, hybridMode;
    public static final float[] BANDGAINS = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f, 2.0f};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_noise_classification_settings);
        getSupportActionBar().setTitle("Noise Classification Settings");

        //Initializing IDs for Noise Classifier and user gains
        initalizeIDs();
        enableIDs();

        // Load values for Noise classification and user gains
        System.loadLibrary("FrequencyDomain");
        loadIDValues();

        vigilanceValue1.addTextChangedListener(new TextWatcher() {
            // the user's changes are saved here
            public void onTextChanged(CharSequence c, int start, int before, int count) {
                if(!c.toString().equals("")) {
                    updateSettingVigilance1(Float.parseFloat(c.toString()));
                }
            }
            public void beforeTextChanged(CharSequence c, int start, int count, int after) {
            }
            public void afterTextChanged(Editable c) {
            }
        });

        vigilanceValue2.addTextChangedListener(new TextWatcher() {
            // the user's changes are saved here
            public void onTextChanged(CharSequence c, int start, int before, int count) {
                if (!c.toString().equals("")) {
                    updateSettingVigilance2(Float.parseFloat(c.toString()));
                }
            }
            public void beforeTextChanged(CharSequence c, int start, int count, int after) {
            }
            public void afterTextChanged(Editable c) {
            }
        });

        FeatAvgBuffVal.addTextChangedListener(new TextWatcher() {
            // the user's changes are saved here
            public void onTextChanged(CharSequence c, int start, int before, int count) {
                if(!c.toString().equals("")) {
                    updateSettingFeatAvgBuffVal(Integer.parseInt(c.toString()));
                }
            }
            public void beforeTextChanged(CharSequence c, int start, int count, int after) {
            }
            public void afterTextChanged(Editable c) {
            }
        });

        NewClassCreationBuffVal.addTextChangedListener(new TextWatcher() {
            // the user's changes are saved here
            public void onTextChanged(CharSequence c, int start, int before, int count) {
                if(!c.toString().equals("")) {
                    updateSettingNewClusterCreaBuffVal(Float.parseFloat(c.toString()));
                }
            }
            public void beforeTextChanged(CharSequence c, int start, int count, int after) {
            }
            public void afterTextChanged(Editable c) {
            }
        });

        DecSmoothingBuffVal.addTextChangedListener(new TextWatcher() {
            // the user's changes are saved here
            public void onTextChanged(CharSequence c, int start, int before, int count) {
                if(!c.toString().equals("")) {
                    updateSettingDecisionSmoothingBuffVal(Float.parseFloat(c.toString()));
                }
            }
            public void beforeTextChanged(CharSequence c, int start, int count, int after) {
            }
            public void afterTextChanged(Editable c) {
            }
        });




        vigilanceValue1.setOnKeyListener(new View.OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN) &&
                        (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    // Perform action on key press
                    updateSettingVigilance1(Float.parseFloat(vigilanceValue1.getText().toString()));
                    return true;
                }
                return false;
            }
        });

        vigilanceValue2.setOnKeyListener(new View.OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN) &&
                        (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    // Perform action on key press
                    updateSettingVigilance2(Float.parseFloat(vigilanceValue2.getText().toString()));
                    return true;
                }
                return false;
            }
        });

        FeatAvgBuffVal.setOnKeyListener(new View.OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN) &&
                        (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    // Perform action on key press
                    updateSettingFeatAvgBuffVal(Integer.parseInt(FeatAvgBuffVal.getText().toString()));
                    return true;
                }
                return false;
            }
        });

        NewClassCreationBuffVal.setOnKeyListener(new View.OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN) &&
                        (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    // Perform action on key press
                    updateSettingNewClusterCreaBuffVal(Float.parseFloat(NewClassCreationBuffVal.getText().toString()));
                    return true;
                }
                return false;
            }
        });

        DecSmoothingBuffVal.setOnKeyListener(new View.OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN) &&
                        (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    // Perform action on key press
                    updateSettingDecisionSmoothingBuffVal(Float.parseFloat(DecSmoothingBuffVal.getText().toString()));
                    return true;
                }
                return false;
            }
        });




        // Set a checked change listener for switch button
        saveData.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setSaveData(isChecked);
            }
        });

        hybridMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setHybridMode(isChecked);
            }
        });

        firstBand.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float Value = BANDGAINS[progress];//getConvertedValue(progress,true);
                updateUserBandGains(0, progress);
                firstBandLabel.setText(String.format("0-500Hz            :%.1f", Value));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        secondBand.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float Value = BANDGAINS[progress];//getConvertedValue(progress,true);
                updateUserBandGains(1, progress);
                secondBandLabel.setText(String.format("500-1000Hz     :%.1f", Value));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        thirdBand.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float Value = BANDGAINS[progress];//getConvertedValue(progress,true);
                updateUserBandGains(2, progress);
                thirdBandLabel.setText(String.format("1000-2000Hz   :%.1f", Value));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        fourthBand.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float Value = BANDGAINS[progress];//getConvertedValue(progress,true);
                updateUserBandGains(3, progress);
                fourthBandLabel.setText(String.format("2000-4000Hz   :%.1f", Value));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        fifthBand.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float Value = BANDGAINS[progress];//getConvertedValue(progress,true);
                updateUserBandGains(4, progress);
                fifthBandLabel.setText(String.format("Above 4000Hz :%.1f", Value));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        //run the handler when it is play audio is on.
        if(getPlayAudioStatus()!=0) {
            disableIDs();
            handler.postDelayed(r, 1000); //(long) (getGuiUpdateRate())

        }

    }



    Handler handler = new Handler();

    final Runnable r = new Runnable() {
        @Override
        public void run() {
            if (getPreviousClusterLabel() != getClusterLabel()) {
                float userBandGains[] = getUserBandGains(getClusterLabel());
                firstBand.setProgress((int) ((userBandGains[0] - 0.1) * 10));
                secondBand.setProgress((int) ((userBandGains[1] - 0.1) * 10));
                thirdBand.setProgress((int) ((userBandGains[2] - 0.1) * 10));
                fourthBand.setProgress((int) ((userBandGains[3] - 0.1) * 10));
                fifthBand.setProgress((int) ((userBandGains[4] - 0.1) * 10));
                firstBandLabel.setText(String.format("0-500Hz            :%.1f", userBandGains[0]));
                secondBandLabel.setText(String.format("500-1000Hz     :%.1f", userBandGains[1]));
                thirdBandLabel.setText(String.format("1000-2000Hz   :%.1f", userBandGains[2]));
                fourthBandLabel.setText(String.format("2000-4000Hz   :%.1f", userBandGains[3]));
                fifthBandLabel.setText(String.format("Above 4000Hz :%.1f", userBandGains[4]));
            }

            /*switch (getVADClass()) {
                case 0:
                    statusView.append("\nQuiet. Noise Classifier Stopped.\n");
                    break;
                case 1: // perform noise classifier and display its results, for only pure noise frames
                    getTime();
                    break;
                case 2:
                    statusView.append("\nPresence of Speech. Noise Classifier Stopped.\n");
                    break;
            }*/


            handler.postDelayed(this, (long) (getGuiUpdateRate() * 1000.0f));
        }
    };

    /*public void getTime(){

        statusView.setMovementMethod(new ScrollingMovementMethod());
        statusView.append(
                "Classified class: " + getClusterLabel() + " out of " + getTotalClusters() + " classes.\n"
        );
        final int scrollAmount = statusView.getLayout().getLineTop(statusView.getLineCount()) - statusView.getHeight();
        if (scrollAmount > 0)
            statusView.scrollTo(0, scrollAmount);
        else
            statusView.scrollTo(0, 0);
    }*/

    @Override
    public boolean onOptionsItemSelected(MenuItem item){
        int id = item.getItemId();

        if (id == android.R.id.home) {
            onBackPressed();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }






    public void loadIDValues(){
        float userBandGains[] = getUserBandGains(getClusterLabel());
        firstBand.setProgress((int) ((userBandGains[0] - 0.1) * 10));
        secondBand.setProgress((int) ((userBandGains[1] - 0.1) * 10));
        thirdBand.setProgress((int) ((userBandGains[2] - 0.1) * 10));
        fourthBand.setProgress((int) ((userBandGains[3] - 0.1) * 10));
        fifthBand.setProgress((int) ((userBandGains[4] - 0.1) * 10));
        firstBandLabel.setText(String.format("0-500Hz            :%.1f", userBandGains[0]));
        secondBandLabel.setText(String.format("500-1000Hz     :%.1f", userBandGains[1]));
        thirdBandLabel.setText(String.format("1000-2000Hz   :%.1f", userBandGains[2]));
        fourthBandLabel.setText(String.format("2000-4000Hz   :%.1f", userBandGains[3]));
        fifthBandLabel.setText(String.format("Above 4000Hz :%.1f", userBandGains[4]));
        vigilanceValue1.setText(String.format("%.2f", getVigilance1()));
        vigilanceValue2.setText(String.format("%.2f", getVigilance2()));
        FeatAvgBuffVal.setText(String.format("%d", getFeatAvgBuffVal()));
        NewClassCreationBuffVal.setText(String.format("%.1f", getNewClusterCreaBuffVal()));
        DecSmoothingBuffVal.setText(String.format("%.1f", getDecisionSmoothingBuffVal()));
        saveData.setChecked((getSaveData() == 1)?true:false);
        hybridMode.setChecked((getHybridMode() == 1)?true:false);
    }


    public void initalizeIDs() {
        firstBand       = (SeekBar)findViewById(R.id.firstBand);
        firstBandLabel  = (TextView) findViewById(R.id.firstBandLabel);
        secondBand      = (SeekBar)findViewById(R.id.secondBand);
        secondBandLabel = (TextView) findViewById(R.id.secondBandLabel);
        thirdBand       = (SeekBar)findViewById(R.id.thirdBand);
        thirdBandLabel  = (TextView) findViewById(R.id.thirdBandLabel);
        fourthBand      = (SeekBar)findViewById(R.id.fourthBand);
        fourthBandLabel = (TextView) findViewById(R.id.fourthBandLabel);
        fifthBand       = (SeekBar)findViewById(R.id.fifthBand);
        fifthBandLabel  = (TextView) findViewById(R.id.fifthBandLabel);
        vigilanceValue1 = (EditText) findViewById(R.id.vigilanceValue1);
        vigilanceValue2 = (EditText) findViewById(R.id.vigilanceValue2);
        FeatAvgBuffVal  = (EditText) findViewById(R.id.FeatAvgBuffVal);
        NewClassCreationBuffVal =  (EditText) findViewById(R.id.NewClassCreationBuffVal);
        DecSmoothingBuffVal =  (EditText) findViewById(R.id.DecSmoothingBuffVal);
        hybridMode      = (Switch) findViewById(R.id.hybridMode);
        saveData        = (Switch) findViewById(R.id.saveData);
        //statusView      = (TextView) findViewById(R.id.statusView); // for showing results of noise classification
    }


    public void enableIDs(){
        hybridMode.setEnabled(true);
        saveData.setEnabled(true);
        vigilanceValue1.setEnabled(true);
        vigilanceValue2.setEnabled(true);
        FeatAvgBuffVal.setEnabled(true);
        NewClassCreationBuffVal.setEnabled(true);
        DecSmoothingBuffVal.setEnabled(true);
    }

    public void disableIDs(){
        hybridMode.setEnabled(false);
        saveData.setEnabled(false);
        vigilanceValue1.setEnabled(false);
        vigilanceValue2.setEnabled(false);
        FeatAvgBuffVal.setEnabled(false);
        NewClassCreationBuffVal.setEnabled(false);
        DecSmoothingBuffVal.setEnabled(false);
    }


    private native void setSaveData(boolean isChecked);
    private native void setHybridMode(boolean isChecked);
    private native int getSaveData();
    private native int getHybridMode();
    private native int getClusterLabel();
    private native int getPreviousClusterLabel();
    private native int getTotalClusters();
    private native int getPlayAudioStatus();
    private native float getGuiUpdateRate();
    private native int getVADClass();

    private native float[] getUserBandGains(int detectedClass);
    private native void updateUserBandGains(int index, int data);
    private native float getVigilance1 (); // from settings.c
    private native float updateSettingVigilance1 (float vigilance1);
    private native float getVigilance2 (); // from settings.c
    private native float updateSettingVigilance2 (float vigilance2);
    private native int getFeatAvgBuffVal (); // from settings.c
    private native int   updateSettingFeatAvgBuffVal (int FeatAvgBufferLength);
    private native float   getNewClusterCreaBuffVal (); // from settings.c
    private native float updateSettingNewClusterCreaBuffVal (float NewClusterCreationBufferSize);
    private native float getDecisionSmoothingBuffVal (); // from settings.c
    private native float updateSettingDecisionSmoothingBuffVal (float DecisionSmoothingBufferSize);
}
