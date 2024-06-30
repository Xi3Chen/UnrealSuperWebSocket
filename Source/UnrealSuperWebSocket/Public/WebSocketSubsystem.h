// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WebSocketProxy.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebSocketSubsystem.generated.h"

/**
 * a websocket subsystem. it provides access to the native websocket library
 */
UCLASS()
class UNREALSUPERWEBSOCKET_API UWebSocketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// connect to a websocket server
	UFUNCTION(BlueprintCallable, Category = "WebSocket|Start", meta = (WorldContext = "WorldConttextObject"))
	UWebSocketProxy* Connect( UObject* WorldConttextObject ,FString Url,const FString& Protocol,bool& success);

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<UWebSocketProxy*> ConnectedWebSockets;
};
