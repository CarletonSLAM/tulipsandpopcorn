package com.example.monty.schedulingapp;

import android.app.TimePickerDialog;
import android.content.Context;
import android.widget.TimePicker;
import android.widget.Toast;

/**
 * Created by Monty on 6/19/2015.
 */
public class TimeSettings implements TimePickerDialog.OnTimeSetListener {
    Context timeContext;
    public TimeSettings(Context context) {
        this.timeContext = context;
    }
    @Override
    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
        Toast.makeText(timeContext, "Selected time is: " + hourOfDay + ":" + minute, Toast.LENGTH_LONG).show();
    }
}
