#ifndef _Gesture_h
#define _Gesture_h

#include "Hand.h"
	
// Acciones a realizar en aplicación Android
#define PLAY          '0'
#define PAUSE         '1'
#define STOP          '2'
#define PLUS_10S      '3'
#define LESS_10S      '4'
#define PLUS_20S      '5' 
#define NO_ACTION     '9'

enum State {
	StarPZ,
	StarNZ,
	StarPY,
	StarPX,
	RockPY,
	RockPZ,
	GoodNZ,
	GoodPZ,
	GoodPX,
	ScissorPY,
	ScissorNZ,
	Default
};
 
enum Input {
	InStarPZ,
	InStarNZ,
	InStarPY,
	InStarPX,
	InRockPY,
	InRockPZ,
	InGoodNZ,
	InGoodPZ,
	InGoodPX,
	InScissorPY,
	InScissorNZ,
	InDefault
};

State currentState = Default;
Input currentInput = InDefault;

class Gesture {
	
	public:
		Hand *hand;
		char action;
		
		// Constructor de la clase Gesture
		Gesture(Hand& hand): hand(&hand) {
			this->action = NO_ACTION;
		}
	
		void updateStateMachine() {
			switch (currentState) {
				case StarPZ: stateStarPZ(); break;
				case StarNZ: stateStarNZ(); break;
				case StarPY: stateStarPY(); break;
				case StarPX: stateStarPX(); break;
				case RockPY: stateRockPY(); break;
				case RockPZ: stateRockPZ(); break;
				case GoodNZ: stateGoodNZ(); break;
				case GoodPZ: stateGoodPZ(); break;
				case GoodPX: stateGoodPX(); break;
				case ScissorPY: stateScissorPY(); break;
				case ScissorNZ: stateScissorNZ(); break;
				case Default: stateDefault(); break;
				}
			}

		void readInput() {

			switch (hand->getMovement()) {
				case (int)SPZ: currentInput = Input::InStarPZ; break;
				case (int)SNZ: currentInput = Input::InStarNZ; break;
				case (int)SPY: currentInput = Input::InStarPY; break;
				case (int)SPX: currentInput = Input::InStarPX; break;
				case (int)RPY: currentInput = Input::InRockPY; break;
				case (int)RPZ: currentInput = Input::InRockPZ; break;
				case (int)GNZ: currentInput = Input::InGoodNZ; break;
				case (int)GPZ: currentInput = Input::InGoodPZ; break;
				case (int)GPX: currentInput = Input::InGoodPX; break;
				case (int)SCPY: currentInput = Input::InScissorPY; break;
				case (int)SCNZ: currentInput = Input::InScissorNZ; break;
				case (int)UNDEFINED: currentInput = Input::InDefault; break;
				default: currentInput = Input::InDefault; break;
			}
		}
		
		char getAction() {
			return this->action;
		}
		
		char getGesture() {
			return this->action;
		}
		
	private:
		void changeState(int newState) {
			currentState = newState;

			switch (currentState) {
				case State::RockPY: outputRockPY();   break;
				case State::GoodNZ: outputGoodNZ();   break;
				case State::GoodPZ: outputGoodPZ();   break;
				case State::GoodPX: outputGoodPX();   break;
				case State::ScissorPY: outputScissorPY();   break;
				case State::ScissorNZ: outputScissorNZ();   break;				
				default: break;
			}
		}
		
		/*
		*	STATES
		*/
		void stateStarPZ() {
			switch(currentInput) {
				case Input::InRockPY: changeState(State::RockPY); break;
				case Input::InGoodNZ: changeState(State::GoodNZ); break;
				case Input::InStarPZ: break;
				default: changeState(State::Default); break;
			}
		}
		
		void stateStarNZ() {
			switch(currentInput) {
				case Input::InGoodPZ: changeState(State::GoodPZ); break;
				case Input::InStarNZ: break;
				default: changeState(State::Default); break;
			}
		}
		
		void stateStarPY() {
			switch(currentInput) {
				case Input::InGoodPX: changeState(State::GoodPX); break;
				case Input::InStarPY: break;
				default: changeState(State::Default); break;
			}
		}
		
		void stateStarPX() {
			switch(currentInput) {
				case Input::InScissorPY: changeState(State::ScissorPY); break;
				case Input::InStarPX: break;
				default: changeState(State::Default); break;
			}
		}
		
		void stateRockPZ() {
			switch(currentInput) {
				case Input::InScissorNZ: changeState(State::ScissorNZ); break;
				case Input::InRockPZ: break;
				default: changeState(State::Default); break;
			}
		}
		
		void stateRockPY() {
			initStates();
		}
		
		void stateGoodNZ() {
			initStates();
		}
		
		void stateGoodPZ() {
			initStates();
		}
		
		void stateGoodPX() {
			initStates();
		}
		
		void stateScissorPY() {
			initStates();
		}
		
		void stateScissorNZ() {
			initStates();
		}
		
		void stateDefault() {
			initStates();
		}

		
		void initStates() {
			switch(currentInput) {
				case Input::InStarPZ: changeState(State::StarPZ); break;
				case Input::InStarNZ: changeState(State::StarNZ); break;
				case Input::InStarPY: changeState(State::StarPY); break;
				case Input::InStarPX: changeState(State::StarPX); break;
				case Input::InRockPZ: changeState(State::RockPZ); break;
				default: changeState(State::Default); break;
			}			
		}

		/*
		*	OUTPUTS
		*/
		void outputRockPY() {
			this->action = STOP;
		}

		void outputGoodNZ() {
			this->action = PLUS_10S;
		}
		
		void outputGoodPZ() {
			this->action = LESS_10S;
		}
		
		void outputGoodPX() {
			this->action = PLAY;
		}
		
		void outputScissorPY() {
			this->action = PLUS_20S;
		}
		
		void outputScissorNZ() {
			this->action = PAUSE;
		}

};

#endif