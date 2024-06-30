// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#ifdef __EMSCRIPTEN__
#define UsingEmsdkWebSocket 1
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#else
#define UsingEmsdkWebSocket 0
#endif // __EMSCRIPTEN__
#include "Tickable.h"
#include "WebSocketProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSocketProxyConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSocketProxyClosed,int32 ,Code, const FString&, Reason, bool, ClosedByPeer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSocketProxyConnectionError, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSocketProxyReceiveString, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSocketProxyReceiveByte, const TArray<uint8>&, Data, int64, BytesRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSocketProxyReceiveByteDone, const TArray<uint8>&, Data);


/**
 * Web Socket Proxy. this is a proxy object that allows you to connect to a websocket server
 */
UCLASS(NotBlueprintable,BlueprintType)
class UNREALSUPERWEBSOCKET_API UWebSocketProxy : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	bool Open(UObject* WorldContextObject ,const FString& url,const FString& Protocol);
	UFUNCTION(BlueprintCallable)
	void Close(int32 SendCloseCode, const FString& SendCloseReason);
	UFUNCTION(BlueprintCallable)
	void SendString(const FString& Data);

	UFUNCTION(BlueprintCallable)
	void SendBytes(TArray<uint8> Data);

	UPROPERTY(BlueprintAssignable,BlueprintReadOnly)
	FSocketProxyConnected OnConnected;
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly)
	FSocketProxyClosed OnClosed;
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly)
	FSocketProxyConnectionError OnConnectionError;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FSocketProxyReceiveString OnReceiveString;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FSocketProxyReceiveByte OnReceiveByte;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FSocketProxyReceiveByteDone OnReceiveByteDone;

	bool ClosedByWebSocketProxy = false;
public:
	//start implementation of FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	//end implementation of FTickableGameObject
private:
	UWorld* CurrentWorld;

	TArray<uint8> Buffer;
#if UsingEmsdkWebSocket
	EMSCRIPTEN_WEBSOCKET_T Socket;

	static EM_BOOL on_open(int eventType, const EmscriptenWebSocketOpenEvent* e, void* userData);
	static EM_BOOL on_message(int eventType, const EmscriptenWebSocketMessageEvent* e, void* userData);
	static EM_BOOL on_close(int eventType, const EmscriptenWebSocketCloseEvent* e, void* userData);
	static EM_BOOL on_error(int eventType, const EmscriptenWebSocketErrorEvent* e, void* userData);
	enum class ECallFunctionName :uint8
	{
		Null,
		Connected,
		Closed,
		ConnectionError,
		TextMessage,
		ByteMessage

	};
	struct FTickEventTragger
	{
		FTickEventTragger(ECallFunctionName InFunctionType, void* InFunctionParams)
			:FunctionType(InFunctionType), FunctionParams(InFunctionParams)
		{}
		ECallFunctionName FunctionType = ECallFunctionName::Null;
		void* FunctionParams = nullptr;
	};
	TArray<FTickEventTragger> TickEventTraggers;
#else
	TSharedPtr<class IWebSocket> Socket = nullptr;
#endif
	void Native_Connected();
	void Native_Closed(int32 Code, const FString& Reason, bool ClosedByPeer);
	void Native_ConnectionError(const FString& Reason);
	void Native_TextMessage(const FString& Message);
	void Native_ByteMessage(const void*  Data, SIZE_T  Size , SIZE_T BytesRemaining);
};
