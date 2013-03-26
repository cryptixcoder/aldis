	package org.alljoyn.bus.sample.chat;

import java.sql.Timestamp;

public class Message 
{
	private long id;
	private String message;
	private Timestamp time;
	
	public Message()
	{
		
	}
	public Message(long id, String message, Timestamp time)
	{
		setId(id);
		setMessage(message);
		setTime(time);
	}
	
	public long getID()
	{
		return id;
	}
	
	public void setId(long id)
	{
		this.id = id;
	}
	
	public String getMessage()
	{
		return message;
	}
	
	public void setMessage(String message)
	{
		this.message = message;
	}
	
	public void setTime(Timestamp time)
	{
		this.time = time;
	}
	
	public Timestamp getTime()
	{
		return time;
	}
	
	@Override
	public String toString()
	{
		return message;
	}
}