#include "ScotlandYardServerGame.h"

#include "EMessages.h"
#include "EMessagesExt.h"
#include "Framework/AssertMessage.h"
#include "Framework/FileHelpers.h"
#include "Framework/Logging/ILogger.h"
#include "Network/EMessages.h"
#include "Network/MessageHelpers.h"
#include "ScotlandYard/Player.h"
#include "ScotlandYard/ScotlandYardGame.h"

//////////////////////////////////////////////////////////////////////////
#include <fstream>
#include "Framework/StringFunctions.h"
#include "ScotlandYard/Map.h"
//////////////////////////////////////////////////////////////////////////

namespace
{
	const std::string g_GameName = "ScotlandYard";

	void RetrievTravelData(RakNet::Packet &a_Packet, uint32_t &a_Destination, ETravelOption &a_TravelOption)
	{
		RakNet::BitStream bitStream(a_Packet.data, a_Packet.length, false);
		bitStream.IgnoreBytes(sizeof(RakNet::MessageID));
		bitStream.IgnoreBytes(sizeof(ClientID));
		bitStream.IgnoreBytes(sizeof(GameID));
		bitStream.Read(a_Destination);
		short travelOption;
		bitStream.Read(travelOption);
		a_TravelOption = static_cast<ETravelOption>(travelOption);
	}

	void WriteTravelLog(const TravelLog &a_TravelLog, RakNet::BitStream &a_Payload)
	{
		uint32_t start = 0;
		uint32_t end = 0;
		ETravelOption travelOption = ETravelOption_Invalid;
		const uint32_t numLogItems = a_TravelLog.GetNumItems();
		a_Payload.Write(numLogItems);
		for (uint32_t index = 0; index < numLogItems; ++index)
		{
			a_TravelLog.GetLogItem(index, start, end, travelOption);
			a_Payload.Write(end);
			a_Payload.Write(static_cast<short>(travelOption));
		}
	}

	uint32_t StringToInt(const std::string &a_String)
	{
		std::stringstream convert(a_String);
		uint32_t result;
		convert >> result;
		return result;
	}
}

ScotlandYardServerGame::ScotlandYardServerGame(GameID a_GameID, RakNet::RakPeerInterface &a_PeerInterface, ILogger &a_Logger)
	: BaseServerGame(a_GameID, a_PeerInterface, a_Logger)
{
	// to delete
	GetServerLogger().WriteLine("ScotlandYardServerGame(): 1");
	std::ifstream fileStream("data/ScotlandYard/map_nodes.txt");
	if (fileStream.is_open())
	{
		GetServerLogger().WriteLine("fileStream.is_open(): success!");
	}
	else
	{
		GetServerLogger().WriteLine("fileStream.is_open(): failed!");
	}
	AssertMessage(fileStream.is_open(), "Unable to open legacy map file!");

	Map m_Map;

	std::string line;
	std::vector<std::string> temp;
	while (std::getline(fileStream, line))
	{
		if (!line.empty())
		{
			std::vector<std::string> tokens = Tokenize(line, ";");
			uint32_t nodeIndex = StringToInt(tokens[0]);
			Node *node = m_Map.m_Nodes[nodeIndex - 1];
			AssertMessage(nullptr != node, "Encounter uninitialized node!");

			temp = Tokenize(tokens[1], ",");
			//InsertEdges(m_Map, nodeIndex, temp, ETravelOption_Taxi);
			temp = Tokenize(tokens[2], ",");
			//InsertEdges(m_Map, nodeIndex, temp, ETravelOption_Bus);
			temp = Tokenize(tokens[3], ",");
			//InsertEdges(m_Map, nodeIndex, temp, ETravelOption_Underground);
			temp = Tokenize(tokens[4], ",");
			//InsertEdges(m_Map, nodeIndex, temp, ETravelOption_Ferry);
			temp = Tokenize(tokens[5], ",");
			node->m_X = StringToInt(temp[0]);
			node->m_Y = StringToInt(temp[1]);
		}
	}

	fileStream.close();
	GetServerLogger().WriteLine("ScotlandYardServerGame(): 2");
	// to delete

	m_Game = new ScotlandYardGame();
	GetServerLogger().WriteLine("ScotlandYardServerGame(): 2");
}

ScotlandYardServerGame::~ScotlandYardServerGame()
{
	delete m_Game;
}

uint32_t ScotlandYardServerGame::GetNumPlayersPerGame()
{
	return ScotlandYardGame::GetNumPlayersPerGame();
}

ClientID ScotlandYardServerGame::WhoseTurnIsIt() const
{
	return GetClient(m_Game->WhoseTurnIsIt());
}

bool ScotlandYardServerGame::HandleGameMessage(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	bool messageHandled = false;
	const unsigned char messageID = static_cast<const unsigned char>(a_Packet.data[0]);
	switch (messageID)
	{
	case EMessage_SendGetMap:
		{
			HandleGetMap(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendGetSpyTravelLog:
		{
			HandleGetSpyTravelLog(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendGetPlayerLocations:
		{
			HandleGetPlayerLocations(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendTravel:
		{
			HandleTravel(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendGetLocation:
		{
			HandleGetLocation(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendGetTravelLog:
		{
			HandleGetTravelLog(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendAmISpy:
		{
			HandleGetAmISpy(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	case EMessage_SendGetRemainingTokens:
		{
			HandleGetRemainingTokens(a_Packet, a_ClientID);
			messageHandled = true;
			break;
		}
	default:
		{
			SendServerErrorMessage(GetPeerInterface(), a_Packet.systemAddress, EServerError_InvalidGameCommand);
			break;
		}
	}
	return messageHandled;
}

std::string ScotlandYardServerGame::GetGameName() const
{
	return g_GameName;
}

void ScotlandYardServerGame::AddPlayer()
{
	const EPlayer newPlayer = static_cast<EPlayer>(m_Game->GetNumPlayers());
	m_Game->AddPlayer(newPlayer);
}

void ScotlandYardServerGame::HandleGetMap(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const std::string mapFile = ReadFileToString(ScotlandYardGame::GetMapFilename());
		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetMap));
		payload.Write(RakNet::RakString(mapFile.c_str()));
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetSpyTravelLog(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const TravelLog& travelLog = m_Game->GetSpyTravelLog();

		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetSpyTravelLog));
		WriteTravelLog(travelLog, payload);

		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetPlayerLocations(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		//const EPlayer currentPlayer = GetPlayer(a_ClientID);
		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetPlayerLocations));

		const ScotlandYardGame &game = *const_cast<ScotlandYardGame*>(m_Game);
		const uint32_t numPlayers = GetNumPlayersPerGame();
		for (uint32_t playerIndex = 0; playerIndex < numPlayers; ++playerIndex)
		{
			EPlayer playerID = static_cast<EPlayer>(playerIndex);
			const Player &player = game.GetPlayer(playerID);
			if (!player.IsSpy())
			{
				payload.Write(playerID);
				payload.Write(GetClient(playerID));
				payload.Write(player.GetPosition());
			}
		}
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleTravel(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const EPlayer player = GetPlayer(a_ClientID);
		if (player == m_Game->WhoseTurnIsIt())
		{
			uint32_t destination;
			ETravelOption travelOption;
			RetrievTravelData(a_Packet, destination, travelOption);
			ETravelResult result = m_Game->Travel(player, destination, travelOption);
			if (ETravelResult_Success == result)
			{
				BroadcastTurnFinished(a_ClientID);
			}
			RakNet::BitStream payload;
			payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvTravelResult));
			payload.Write(static_cast<short>(result));
			SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
		}
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetLocation(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const EPlayer currentPlayer = GetPlayer(a_ClientID);
		const ScotlandYardGame &game = *const_cast<ScotlandYardGame*>(m_Game);
		const Player &player = game.GetPlayer(currentPlayer);

		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetLocation));
		payload.Write(player.GetPosition());
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetTravelLog(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const EPlayer currentPlayer = GetPlayer(a_ClientID);
		const ScotlandYardGame &game = *const_cast<ScotlandYardGame*>(m_Game);
		const Player &player = game.GetPlayer(currentPlayer);
		const TravelLog &travelLog = player.GetTravelLog();

		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetTravelLog));
		WriteTravelLog(travelLog, payload);
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetAmISpy(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const EPlayer currentPlayer = GetPlayer(a_ClientID);
		const ScotlandYardGame &game = *const_cast<ScotlandYardGame*>(m_Game);
		const Player &player = game.GetPlayer(currentPlayer);

		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvAmISpy));
		payload.Write(player.IsSpy());
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

void ScotlandYardServerGame::HandleGetRemainingTokens(RakNet::Packet &a_Packet, ClientID a_ClientID)
{
	if (m_Game->IsActive())
	{
		const EPlayer currentPlayer = GetPlayer(a_ClientID);
		const ScotlandYardGame &game = *const_cast<ScotlandYardGame*>(m_Game);
		const Player &player = game.GetPlayer(currentPlayer);

		RakNet::BitStream payload;
		payload.Write(static_cast<RakNet::MessageID>(EMessage_RecvGetRemainingTokens));
		payload.Write(static_cast<short>(ETravelOption_Taxi));
		payload.Write(player.GetTokens(ETravelOption_Taxi));
		payload.Write(static_cast<short>(ETravelOption_Bus));
		payload.Write(player.GetTokens(ETravelOption_Bus));
		payload.Write(static_cast<short>(ETravelOption_Underground));
		payload.Write(player.GetTokens(ETravelOption_Underground));
		if (player.IsSpy())
		{
			payload.Write(static_cast<short>(ETravelOption_Black));
			payload.Write(player.GetTokens(ETravelOption_Black));
		}
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, payload);
	}
	else
	{
		SendMessage(GetPeerInterface(), a_Packet.systemAddress, EMessage_RecvGameNotActive);
	}
}

