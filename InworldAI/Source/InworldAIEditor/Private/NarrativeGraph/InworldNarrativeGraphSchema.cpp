// Copyright Epic Games, Inc. All Rights Reserved.

#include "NarrativeGraph/InworldNarrativeGraphSchema.h"

#include "NarrativeGraph/InworldNarrativeGraphNode.h"

#include "InworldStudioTypes.h"

#define LOCTEXT_NAMESPACE "InworldNarrativeSchema"

class INarrativeGraphNodeCreatorInterface
{
public:
	virtual ~INarrativeGraphNodeCreatorInterface() = default;
	virtual UInworldNarrativeGraphNode* CreateNode() = 0;
	virtual void Finalize() = 0;
};

template<class T>
class NarrativeGraphNodeCreator : public INarrativeGraphNodeCreatorInterface
{
public:
	NarrativeGraphNodeCreator(UEdGraph& InGraph)
		: NodeCreator(InGraph)
	{}
	virtual ~NarrativeGraphNodeCreator() = default;

	virtual UInworldNarrativeGraphNode* CreateNode() override { return NodeCreator.CreateNode(); }
	virtual void Finalize() override { NodeCreator.Finalize(); }
private:
	FGraphNodeCreator<T> NodeCreator;
};

void UInworldNarrativeGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FInworldStudioGraphData GraphData;
	GraphData.DisplayName = "Test";
	GraphData.Name = "GRAPH_Test";

	FInworldStudioGraphNodeData Start;
	Start.bIsStart = true;
	Start.Scene = "";
	Start.Name = "Start";
	
	FInworldStudioGraphNodeData House;
	House.bIsStart = false;
	House.Scene = "Bob and Linda are at home, they are feeling hungry.";
	House.Name = "House";

	FInworldStudioGraphNodeData Car;
	Car.bIsStart = false;
	Car.Scene = "Bob and Linda are in the car, they are listening to the radio.";
	Car.Name = "Car";

	FInworldStudioGraphNodeData GasStation;
	GasStation.bIsStart = false;
	GasStation.Scene = "The Gas Station is a great place to refuel a car.";
	GasStation.Name = "GasStation";

	FInworldStudioGraphNodeData Store;
	Store.bIsStart = false;
	Store.Scene = "Bob and Linda are at the store, they are looking for snacks to buy.";
	Store.Name = "Store";

	GraphData.Nodes = { Start, House, Car, GasStation, Store };

	FInworldStudioGraphConnectionData StartToHouse;
	StartToHouse.Name = "Start To House";
	StartToHouse.From = "Start";
	StartToHouse.To = "House";
	StartToHouse.Text = "Begin at House";

	FInworldStudioGraphConnectionData HouseToCar;
	HouseToCar.Name = "House To Car";
	HouseToCar.From = "House";
	HouseToCar.To = "Car";
	HouseToCar.Text = "House to Car";

	FInworldStudioGraphConnectionData CarToHouse;
	CarToHouse.Name = "Car To House";
	CarToHouse.From = "Car";
	CarToHouse.To = "House";
	CarToHouse.Text = "Car to House";

	FInworldStudioGraphConnectionData GasStationToCar;
	GasStationToCar.Name = "GasStation To Car";
	GasStationToCar.From = "GasStation";
	GasStationToCar.To = "Car";
	GasStationToCar.Text = "GasStation to Car";

	FInworldStudioGraphConnectionData CarToGasStation;
	CarToGasStation.Name = "Car To GasStation";
	CarToGasStation.From = "Car";
	CarToGasStation.To = "GasStation";
	CarToGasStation.Text = "Car to GasStation";

	FInworldStudioGraphConnectionData CarToStore;
	CarToStore.Name = "Car To Store";
	CarToStore.From = "Car";
	CarToStore.To = "Store";
	CarToStore.Text = "Car to Store";

	FInworldStudioGraphConnectionData StoreToCar;
	StoreToCar.Name = "Store To Car";
	StoreToCar.From = "Store";
	StoreToCar.To = "Car";
	StoreToCar.Text = "Store to Car";

	GraphData.Connections = { StartToHouse, HouseToCar, CarToHouse, CarToGasStation, GasStationToCar, CarToStore, StoreToCar };

	const int32 NodeHeightOffset = -50;
	const int32 NodeWidthOffset = 150;

	int32 NumFoundThisLevel = 0;
	TSet<FInworldStudioGraphNodeData*> Found;
	TQueue<FInworldStudioGraphNodeData*> ToProcess;
	TMap<FString, UInworldNarrativeGraphNode*> NameToNodeMap;
	TMap<FString, FInworldStudioGraphNodeData*> NameToNodeDataMap;
	TMap<FInworldStudioGraphNodeData*, TArray<FInworldStudioGraphConnectionData*>> NodeDataFromToConnectionMap;
	TMap<FInworldStudioGraphNodeData*, TArray<FInworldStudioGraphConnectionData*>> NodeDataToFromConnectionMap;
	for (auto& Node : GraphData.Nodes)
	{
		if (Node.bIsStart)
		{
			NumFoundThisLevel++;
			ToProcess.Enqueue(&Node);
			Found.Add(&Node);
		}
		NameToNodeDataMap.Add(Node.Name, &Node);
		NodeDataFromToConnectionMap.Add(&Node, {});
		NodeDataToFromConnectionMap.Add(&Node, {});
	}
	for (auto& Connection : GraphData.Connections)
	{
		NodeDataFromToConnectionMap[NameToNodeDataMap[Connection.From]].Add(&Connection);
		NodeDataToFromConnectionMap[NameToNodeDataMap[Connection.To]].Add(&Connection);
	}

	int32 CurrentLevel = 0;
	while (!ToProcess.IsEmpty())
	{
		const int32 NumToProcess = NumFoundThisLevel;
		NumFoundThisLevel = 0;
		for (int32 i = 0; i < NumToProcess; ++i)
		{
			FInworldStudioGraphNodeData* NodeData = nullptr;
			ToProcess.Dequeue(NodeData);
			for (FInworldStudioGraphConnectionData* Connection : NodeDataFromToConnectionMap[NodeData])
			{
				FInworldStudioGraphNodeData* To = NameToNodeDataMap[Connection->To];
				if (Found.Contains(To)) continue;
				Found.Add(To);
				NumFoundThisLevel++;
				ToProcess.Enqueue(To);
			}

			TUniquePtr<INarrativeGraphNodeCreatorInterface> NodeCreator = nullptr;
			if (NodeData->bIsStart)
			{
				NodeCreator = MakeUnique<NarrativeGraphNodeCreator<UInworldNarrativeGraphNode_Root>>(Graph);
			}
			else
			{
				NodeCreator = MakeUnique<NarrativeGraphNodeCreator<UInworldNarrativeGraphNode_Scene>>(Graph);
			}

			UInworldNarrativeGraphNode* Node = NodeCreator->CreateNode();
			Node->Name = NodeData->Name;
			Node->Scene = NodeData->Scene;
			for (const auto& Connection : NodeDataFromToConnectionMap[NodeData])
			{
				Node->Outputs.Add(Connection->To);
			}
			for (const auto& Connection : NodeDataToFromConnectionMap[NodeData])
			{
				Node->Inputs.Add(Connection->From);
			}
			Node->NodePosY = NodeHeightOffset * i;
			Node->NodePosX = NodeWidthOffset * CurrentLevel;
			NodeCreator->Finalize();
			SetNodeMetaData(Node, FNodeMetadata::DefaultGraphNode);
			NameToNodeMap.Add(NodeData->Name, Node);
		}
		CurrentLevel++;
	}
	for (auto& Connection : GraphData.Connections)
	{
		UInworldNarrativeGraphNode* From = NameToNodeMap[Connection.From];
		UInworldNarrativeGraphNode* To = NameToNodeMap[Connection.To];
		TryCreateConnection(From->Outputs[To->Name], To->Inputs[From->Name]);
	}
}

const FPinConnectionResponse UInworldNarrativeGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("ConnectResponse_Allowed", "Connect"));
}

#undef LOCTEXT_NAMESPACE
