package org.alljoyn.bus.sample.chat;

// See http://www.vogella.com/articles/AndroidSQLite/article.html

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteStatement;
import android.util.Log;
public class MessageDAO 
{
	private final static String TAG = "MessageDAO: ";
	private SQLiteDatabase db;
	private MySQLiteHelper dbHelper;
	private String[] allColumns = { MySQLiteHelper.COLUMN_ID,
			MySQLiteHelper.COLUMN_MESSAGE, MySQLiteHelper.COLUMN_TIMESTAMP };
	
	
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
        
        // Delete if too many messages
        while (getNumMessages() > MySQLiteHelper.MESSAGE_LIMIT)
        	removeOldest();
        
        Date date = new Date();
        
		ContentValues values = new ContentValues();
		values.put(MySQLiteHelper.COLUMN_MESSAGE, message);		
		values.put(MySQLiteHelper.COLUMN_TIMESTAMP, date.getTime());
		
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
		
		String orderBy = MySQLiteHelper.COLUMN_TIMESTAMP + " desc";
		Cursor cursor = db.query(MySQLiteHelper.TABLE_NAME,	allColumns, null, null, null, null, orderBy, null);
		
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
	
	public List<String> getAllMessageStrings()
	{
		List<Message> messageList = getAllMessages();
		
		List<String> stringList = new ArrayList<String>();
		
		for(Message message : messageList)
		{
			stringList.add(message.toString());
		}
		
		return stringList;
	}
	
	public void deleteAllMessages()
	{
		db.delete(MySQLiteHelper.TABLE_NAME, null, null);
	}
	
	/* Private functions */
	
	private Message cursorToMessage(Cursor cursor)
	{
		Message message = new Message();
		message.setId(cursor.getLong(0));
		message.setMessage(cursor.getString(1));
		message.setTime(new Timestamp(cursor.getLong(2)));		
		
		return message;
	}
	
	private int getNumMessages()
	{
		return (int) DatabaseUtils.queryNumEntries(db, MySQLiteHelper.TABLE_NAME);
	}
	
	private void removeOldest()
	{
		Log.i(TAG, "Number before delete " + getNumMessages());
		SQLiteStatement	 deleteSQL = db.compileStatement(MySQLiteHelper.DELETE_QUERY);
		
		deleteSQL.execute();
		Log.i(TAG, "Number after delete " + getNumMessages());

	}
}
