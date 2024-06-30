// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocketSubsystem.h"

UWebSocketProxy* UWebSocketSubsystem::Connect(UObject* WorldConttextObject,FString Url, const FString& Protocol, bool& success)
{
	if (WorldConttextObject == nullptr)
	{
		success = false;
		return nullptr;
	}
	UWebSocketProxy* proxy = NewObject<UWebSocketProxy>();
	if(proxy->Open(WorldConttextObject,Url,Protocol))
	{
		ConnectedWebSockets.Add(proxy);
		success = true;
		return proxy;
	}
	return nullptr;
}
