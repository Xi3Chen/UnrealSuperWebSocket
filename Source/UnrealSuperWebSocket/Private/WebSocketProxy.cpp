// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocketProxy.h"
 
#if UsingEmsdkWebSocket
#include <string>
#else
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#endif 


bool UWebSocketProxy::Open(UObject* WorldContextObject ,const FString& url, const FString& Protocol)
{
#if UsingEmsdkWebSocket
	if (WorldContextObject)
	{
		CurrentWorld = WorldContextObject->GetWorld();

	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("\nWorldContextObject is not valid\n"));
		return false;
	}
	if (CurrentWorld == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("\nWorldContextObject is not valid\n"));
		return false;
	}
	if (!emscripten_websocket_is_supported()) {
		UE_LOG(LogTemp, Log, TEXT("\nemscripten_websocket not supoported :/\n"));
		return false;
	}

	std::string LocalUrl = TCHAR_TO_UTF8(*url);
	EmscriptenWebSocketCreateAttributes ws_attrs = {
		LocalUrl.c_str() ,
		NULL,
		EM_TRUE
	};

	Socket = emscripten_websocket_new(&ws_attrs);
	if (Socket <= 0) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create WebSocket."));
		return false;
	}

	emscripten_websocket_set_onopen_callback(Socket, this, on_open);
	emscripten_websocket_set_onerror_callback(Socket, this, on_error);
	emscripten_websocket_set_onclose_callback(Socket, this, on_close);
	emscripten_websocket_set_onmessage_callback(Socket, this, on_message);
#else

	if (WorldContextObject)
	{
		CurrentWorld = GEngine->GetWorldFromContextObject(WorldContextObject);
	}
	if (Socket.IsValid())
	{
		return false;
	}
	FWebSocketsModule& WebSocketsModuleRef = FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));

	Socket = WebSocketsModuleRef.Get().CreateWebSocket(url, Protocol.ToLower());
	Socket->OnConnected().AddUObject(this, &UWebSocketProxy::Native_Connected);
	Socket->OnClosed().AddUObject(this, &UWebSocketProxy::Native_Closed);
	Socket->OnConnectionError().AddUObject(this, &UWebSocketProxy::Native_ConnectionError);
	Socket->OnMessage().AddUObject(this, &UWebSocketProxy::Native_TextMessage);
	Socket->OnRawMessage().AddUObject(this, &UWebSocketProxy::Native_ByteMessage);
	Socket->Connect();
	
#endif
	
	

	return true;
}
void UWebSocketProxy::Close(int32 SendCloseCode, const FString& SendCloseReason)
{
#if UsingEmsdkWebSocket
	std::string LocalSendCloseReason = TCHAR_TO_UTF8(*SendCloseReason);
	emscripten_websocket_close(Socket, (unsigned short)SendCloseCode, LocalSendCloseReason.c_str());
#else
	if (Socket)
	{
	
		Socket->Close(SendCloseCode, SendCloseReason);
		}
#endif
}

void UWebSocketProxy::SendString(const FString& Data)
{
#if UsingEmsdkWebSocket
	if (Socket > 0 && Data.Len() > 0)
	{
		std::string LocalData = TCHAR_TO_UTF8(*Data);
		emscripten_websocket_send_utf8_text(Socket, LocalData.c_str());
	}
#else
	if (Socket && Socket->IsConnected() && Data.Len() > 0)
	{
		Socket->Send(Data);
	}
#endif // UsingEmsdkWebSocket

}

void UWebSocketProxy::SendBytes(TArray<uint8> Data)
{
#if UsingEmsdkWebSocket
	if (Socket > 0 && Data.Num() > 0)
	{
		emscripten_websocket_send_binary(Socket, Data.GetData(), Data.Num());
	}
#else
	if (Socket && Socket->IsConnected() && Data.Num() > 0)
	{
		Socket->Send(Data.GetData(),Data.Num(),true);
	}
#endif // UsingEmsdkWebSocket

}

void UWebSocketProxy::Tick(float DeltaTime)
{
#if UsingEmsdkWebSocket
	for (auto& TickEventTragger : TickEventTraggers)
	{
		switch (TickEventTragger.FunctionType)
		{
		case ECallFunctionName::Connected:
			OnConnected.Broadcast();
			break;
		case ECallFunctionName::Closed:
		{
			TTuple<int32, FString, bool>* Data = (TTuple<int32, FString, bool>*)TickEventTragger.FunctionParams;
			OnClosed.Broadcast(Data->Get<0>(), Data->Get<1>(), Data->Get<2>());
			delete Data;
			TickEventTragger.FunctionParams = nullptr;
		}
			break;
		case ECallFunctionName::ConnectionError:
			OnConnectionError.Broadcast(FString());
			break;
		case ECallFunctionName::TextMessage:
		{
			FString* Data = (FString*)TickEventTragger.FunctionParams;

			OnReceiveString.Broadcast(*Data);
			delete Data;
			TickEventTragger.FunctionParams = nullptr;
		}
			break;
		case ECallFunctionName::ByteMessage:
		{
			TTuple<TArray<uint8>, int32>* Data = (TTuple<TArray<uint8>, int32>*)TickEventTragger.FunctionParams;
			OnReceiveByte.Broadcast(Data->Get<0>(), Data->Get<1>());
			OnReceiveByteDone.Broadcast(Data->Get<0>());
			delete Data;
			TickEventTragger.FunctionParams = nullptr;
		}
			
			break;
		case ECallFunctionName::Null:
			break;
		}
	}
	TickEventTraggers.Empty();
#endif
}
bool UWebSocketProxy::IsTickable() const
{
#if UsingEmsdkWebSocket
	return true;
#else
	return false;

#endif // UsingEmsdkWebSocket

}
TStatId UWebSocketProxy::GetStatId() const
{
	return TStatId();
}
#if UsingEmsdkWebSocket

EM_BOOL UWebSocketProxy::on_open(int eventType, const EmscriptenWebSocketOpenEvent* e, void* userData)
{
	UWebSocketProxy* self = (UWebSocketProxy*)userData;
	new (self->TickEventTraggers)FTickEventTragger(ECallFunctionName::Connected, nullptr);

	return EM_TRUE;
}

EM_BOOL UWebSocketProxy::on_message(int eventType, const EmscriptenWebSocketMessageEvent* e, void* userData)
{
	if (e->isText)
	{
		UWebSocketProxy* self = (UWebSocketProxy*)userData;
		std::string message(reinterpret_cast<const char*>(e->data), e->numBytes);
		FString LocalString = FString(UTF8_TO_TCHAR(message.c_str()));
		FString* Message = new FString(LocalString);
		//delete CStr;
		new (self->TickEventTraggers)FTickEventTragger(ECallFunctionName::TextMessage, Message);
	}
	else
	{
		UWebSocketProxy* self = (UWebSocketProxy*)userData;
		int32 size = e->numBytes;
		TTuple<TArray<uint8>, int32>* Data = new TTuple<TArray<uint8>, int32>(TArray<uint8>((uint8*)e->data, e->numBytes), size);
		new (self->TickEventTraggers)FTickEventTragger(ECallFunctionName::ByteMessage, Data);
	}
	return EM_TRUE;
}

EM_BOOL UWebSocketProxy::on_close(int eventType, const EmscriptenWebSocketCloseEvent* e, void* userData)
{
	UWebSocketProxy* self = (UWebSocketProxy*)userData;
	int32 CloseCode = e->code;
	bool ClosedByPeer = (!self->ClosedByWebSocketProxy) && e->wasClean != self->ClosedByWebSocketProxy;

	TTuple<int32, FString, bool>* Data = new TTuple<int32, FString, bool>(CloseCode, FString(e->reason), ClosedByPeer);
	return EM_TRUE;
}

EM_BOOL UWebSocketProxy::on_error(int eventType, const EmscriptenWebSocketErrorEvent* e, void* userData)
{
	UWebSocketProxy* self = (UWebSocketProxy*)userData;
	
	new (self->TickEventTraggers)FTickEventTragger(ECallFunctionName::ConnectionError, nullptr);
	return EM_TRUE;
}


#endif
void UWebSocketProxy::Native_Connected()
{
	OnConnected.Broadcast();
}

void UWebSocketProxy::Native_Closed(int32 Code, const FString& Reason, bool ClosedByPeer)
{
	OnClosed.Broadcast(Code, Reason, ClosedByPeer);
}

void UWebSocketProxy::Native_ConnectionError(const FString& Reason)
{
	OnConnectionError.Broadcast(Reason);
}

void UWebSocketProxy::Native_TextMessage(const FString& Message)
{
	OnReceiveString.Broadcast(Message);
}

void UWebSocketProxy::Native_ByteMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	if (Size > 0)
	{
		TArray<uint8> LocalBuffer;
		LocalBuffer.Append((uint8*)Data, Size);
		OnReceiveByte.Broadcast(LocalBuffer, BytesRemaining);

		Buffer.Append((uint8*)Data,Size);
	}
	if (BytesRemaining == 0)
	{
		OnReceiveByteDone.Broadcast(Buffer);
		Buffer.Empty();
	}
}