package com.example.monty.schedulingapp;

import android.provider.BaseColumns;

/**
 * Created by Monty on 6/20/2015.
 */
public class TableData {

    public TableData() {

    }

    public static abstract class TableInfo implements BaseColumns {
        DateHandler date = new DateHandler();
        public final int YEAR = date.year;
        public final int MONTH = date.month;
        public final int DAY = date.day;

        TimeHandler time = new TimeHandler();
        public final int HOUR = time.getHour();
        public final int MINUTE = time.getMinute();

        public static final String DATABASE_NAME = "dbase";
        public static final String TABLE_NAME = "schedule";
    }
}
