package com.example.monty.schedulingapp;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;

import java.util.Calendar;

/**
 * Created by Monty on 6/19/2015.
 */
public class TimeHandler extends DialogFragment {

    public int hour;
    public int minute;

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Calendar timeCalendar = Calendar.getInstance();
        hour = timeCalendar.get(Calendar.HOUR_OF_DAY);
        minute = timeCalendar.get(Calendar.MINUTE);
        TimePickerDialog timeDialog;
        TimeSettings timeSettings = new TimeSettings(getActivity());
        timeDialog = new TimePickerDialog(getActivity(), timeSettings, hour, minute, true);
        return timeDialog;

        //return super.onCreateDialog(savedInstanceState);
    }

    public int getHour() {
        return hour;
    }

    public int getMinute() {
        return minute;
    }
}