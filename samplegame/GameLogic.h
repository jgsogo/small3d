#ifndef GAMELOGIC_H_
#define GAMELOGIC_H_

#include <memory>
#include <dimitrikourk/small3d/Configuration.h>
#include <dimitrikourk/small3d/EngineLog.h>
#include <dimitrikourk/small3d/SceneObject.h>
#include "KeyInput.h"
#include <dimitrikourk/small3d/Scene.h>

using namespace small3d;

namespace AvoidTheBug3D {

	/**
	 * @class	GameLogic
	 *
	 * @brief	Game logic
	 *
	 * @author	Dimitri Kourkoulis
	 * @date	Dec 26, 2013
	 */

	class GameLogic {

	private:

		shared_ptr<Configuration> cfg;
		shared_ptr<EngineLog> log;

		shared_ptr<SceneObject> goat;
		shared_ptr<SceneObject> bug;
		shared_ptr<SceneObject> tree;

		enum GameState {START_SCREEN, PLAYING};
		GameState gameState;

		enum BugState {FLYING_STRAIGHT, TURNING, DIVING_DOWN, DIVING_UP};
		BugState bugState, bugPreviousState;
		int bugFramesInCurrentState;
		float bugVerticalSpeed;

		void initGame();
		void processGame(const KeyInput &keyInput);
		void processStartScreen(const KeyInput &keyInput);

		void moveGoat(const KeyInput &keyInput);
		void moveBug();
		
	public:

		/**
		 * @brief	The game scene.
		 */
		shared_ptr<Scene> gameScene;

		/**
		 * Constructor
		 * @param	cfg	The game's configuration object.
		 * @param	log	The game's logging object.
		 */
		GameLogic(const shared_ptr<Configuration> cfg,
			const shared_ptr<EngineLog> log);

		/**
		* Destructor
		*/
		~GameLogic();

		/**
		 * Process conditions and set up the next frame, also taking into consideration
		 * the input from the keyboard
		 * 
		 * @param	keyInput	The keyboard input
		 */
		void process(const KeyInput &keyInput);
	};

} /* namespace AvoidTheBug3D */

#endif /* GAMELOGIC_H_ */