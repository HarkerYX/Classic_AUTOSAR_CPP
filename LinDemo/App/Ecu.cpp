/*
 * Ecu.cpp
 *
 *  Created on: Mar 27, 2019
 *      Author: messam
 */
#include "Ecu.h"
#include <thread>
#include <chrono>

namespace Ecu{
Ecu::Ecu(){
	LinPtr = std::make_shared<Lin::Lin>();
	LinIfPtr = std::make_shared<LinIf::LinIf>();
	LinSMPtr = std::make_shared<LinSM::LinSM>();
	ComMPtr = std::make_shared<ComM::ComM>();
	PduRPtr = std::make_shared<PduR::PduR>();
	LdComPtr = std::make_shared<LdCom::LdCom>();
	BswMPtr = std::make_shared<BswM::BswM>();
	LinPtr->Init(std::bind(&Ecu::Ecu::LinConfigure, this));
	LinIfPtr->Init(std::bind(&Ecu::Ecu::LinIfConfigure, this, std::placeholders::_1), LinPtr, LinSMPtr, PduRPtr);
	LinSMPtr->Init(std::bind(&Ecu::Ecu::LinSMConfigure, this, std::placeholders::_1), LinIfPtr, ComMPtr, BswMPtr);
	ComMPtr->Init(std::bind(&Ecu::Ecu::ComMConfigure, this, std::placeholders::_1), LinSMPtr, nullptr, nullptr, nullptr);
	PduRPtr->Init(std::bind(&Ecu::Ecu::PduRConfigure, this), LinIfPtr, LdComPtr);
	LdComPtr->Init(std::bind(&Ecu::Ecu::LdComConfigure, this), PduRPtr, AppPtr);
	BswMPtr->Init(LinSMPtr, ComMPtr);
}

void Ecu::Execute(){
	std::thread thread(&Ecu::Scheduler, this);
	thread.detach();

	ComMPtr->CommunicationAllowed(0, true);
	ComMPtr->RequestComMode(0, ComM::ComM_ModeType::COMM_FULL_COMMUNICATION);
	while(1){
		static uint16 Counter = 0;
		Counter++;

		if(Counter == 5){
			LinSMPtr->ScheduleRequest(0, 0);
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void Ecu::Scheduler(){
	while(1){
		LinIfPtr->MainFunction();
		LinSMPtr->MainFunction();
		ComMPtr->MainFunction(0);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

}


