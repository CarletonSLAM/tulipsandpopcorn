package com.example.monty.schedulingapp;

import android.app.DatePickerDialog;
import android.content.Context;
import android.widget.DatePicker;
import android.widget.Toast;

/**
 * Created by Monty on 6/20/2015.
 */
public class DateSettings implements DatePickerDialog.OnDateSetListener {
    Context dateContext;
    public DateSettings(Context dateContext) {
        this.dateContext = dateContext;
    }
    @Override
    public void onDateSet(DatePicker view, int year, int monthOfYear, int dayOfMonth) {
        Toast.makeText(dateContext, "Selected date is: " + dayOfMonth + "/" + monthOfYear + "/" + year, Toast.LENGTH_LONG).show();
    }
}
