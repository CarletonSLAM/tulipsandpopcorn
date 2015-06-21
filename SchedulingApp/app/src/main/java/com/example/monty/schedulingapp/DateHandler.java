package com.example.monty.schedulingapp;

import android.app.DatePickerDialog;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;

import java.util.Calendar;

/**
 * Created by Monty on 6/20/2015.
 */
public class DateHandler extends DialogFragment {

    public int year;
    public int month;
    public int day;
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Calendar dateCalendar = Calendar.getInstance();
        year = dateCalendar.get(Calendar.YEAR);
        month = dateCalendar.get(Calendar.MONTH);
        day = dateCalendar.get(Calendar.DAY_OF_MONTH);
        DatePickerDialog dateDialog;
        DateSettings dateSettings = new DateSettings(getActivity());
        dateDialog = new DatePickerDialog(getActivity(), dateSettings, year, month, day);
        return dateDialog;

        //return super.onCreateDialog(savedInstanceState);
    }
}
