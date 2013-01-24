	package org.alljoyn.bus.sample.chat;

public class Message 
{
	private long id;
	private String message;
	
	public Message()
	{
		
	}
	public Message(long id, String message)
	{
		setId(id);
		setMessage(message);
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
	
	@Override
	public String toString()
	{
		return message;
	}
}