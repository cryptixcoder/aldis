package org.alljoyn.bus.sample.chat;

// See http://www.vogella.com/articles/AndroidSQLite/article.html

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;
public class MessageDAO 
{
	private final static String TAG = "MessageDAO: ";
	private SQLiteDatabase db;
	private MySQLiteHelper dbHelper;
	private String[] allColumns = { MySQLiteHelper.COLUMN_ID,
			MySQLiteHelper.COLUMN_MESSAGE };
	
	
	public MessageDAO(Context context)
	{
		dbHelper = new MySQLiteHelper(context);
	}
	
	public void open() throws SQLException
	{
		db = dbHelper.getWritableDatabase();
	}
	
	public void close()
	{
		dbHelper.close();
	}
	
	public Message createMessage(String message)
	{
        Log.i(TAG, "Attempting to insert " + message);
		ContentValues values = new ContentValues();
		values.put(MySQLiteHelper.COLUMN_MESSAGE, message);
		long id = db.insert(MySQLiteHelper.TABLE_NAME, null, values);
		
		Cursor cursor = db.query(MySQLiteHelper.TABLE_NAME,
				allColumns, MySQLiteHelper.COLUMN_ID + " =  " + id ,null,
				null, null, null);
		cursor.moveToFirst();
		Message newMessage = cursorToMessage(cursor);
		cursor.close();
		return newMessage;
	}
	
	public List<Message> getAllMessages() 
	{
		List<Message> messages = new ArrayList<Message>();
		
		Cursor cursor = db.query(MySQLiteHelper.TABLE_NAME,
				allColumns, null, null, null, null, null);
		
		cursor.moveToFirst();
		while (!cursor.isAfterLast())
		{
			Message message = cursorToMessage(cursor);
			messages.add(message);
			cursor.moveToNext();		
		}
		cursor.close();
		return messages;
				
	}
	private Message cursorToMessage(Cursor cursor)
	{
		Message message = new Message();
		message.setId(cursor.getLong(0));
		message.setMessage(cursor.getString(1));
		return message;
	}
}
