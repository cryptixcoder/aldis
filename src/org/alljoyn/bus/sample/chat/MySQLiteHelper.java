package org.alljoyn.bus.sample.chat;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class MySQLiteHelper extends SQLiteOpenHelper 
{
	public static final String TABLE_NAME = "messages";
	public static final String COLUMN_ID = "id";
	public static final String COLUMN_MESSAGE = "message";
	public static final String COLUMN_TIMESTAMP = "timestamp";
	public static final int MESSAGE_LIMIT = 10;

	
	private static final String DATABASE_NAME = "messages.db";
	private static final int DATABASE_VERSION = 2;
	
	private final static String CREATE_QUERY = "create table "
			+ TABLE_NAME + "(" 
			+ COLUMN_ID + " integer primary key autoincrement, " 
			+ COLUMN_MESSAGE + " text not null, "
			+ COLUMN_TIMESTAMP + " integer not null default (strftime('%s','now')));";
		
	
	//delete from messages where id = (select id from messages where timestamp = (select min(timestamp) from messages));
	public final static String DELETE_QUERY = "delete from "
			+ TABLE_NAME
			+ " where " + COLUMN_ID + " = (select id from messages where timestamp = (select min(timestamp) from messages))";
	
	public MySQLiteHelper(Context context)
	{
		super(context, DATABASE_NAME, null, DATABASE_VERSION);
	}
	
	@Override
	public void onCreate(SQLiteDatabase db) 
	{
		db.execSQL(CREATE_QUERY);
	}

	@Override
	 public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
	{
		db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAME);
	    onCreate(db);
	}
}