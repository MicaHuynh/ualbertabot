#include "Player_Kiter.h"

using namespace SparCraft;

Player_Kiter::Player_Kiter (const size_t & playerID) 
{
	_playerID = playerID;
}

void Player_Kiter::getMove(const GameState & state, Move & move)
{
    MoveArray moves;
    ActionGenerators::GenerateCompassActions(state, _playerID, moves);

    move.clear();
	for (size_t u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		size_t furthestMoveIndex				(0);
		size_t furthestMoveDist					(0);
		size_t closestMoveIndex					(0);
		int actionDistance						(std::numeric_limits<int>::max());
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? AITools::GetClosestOurUnit(state, _playerID, u) : AITools::GetClosestEnemyUnit(state, _playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Action move						(moves.getMove(u, m));
				
			if (move.type() == ActionTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.getPlayerID()), move.getTargetID()));
				PositionType dist				(ourUnit.getDistanceSqToUnit(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == ActionTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.getPlayerID(), move.getTargetID()));
				PositionType dist				(ourUnit.getDistanceSqToUnit(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == ActionTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Constants::Move_Dir[move.getTargetID()][0], 
												 ourUnit.y() + Constants::Move_Dir[move.getTargetID()][1]);
				size_t dist						(closestUnit.getDistanceSqToPosition(ourDest, state.getTime()));

				if (dist > furthestMoveDist)
				{
					furthestMoveDist = dist;
					furthestMoveIndex = m;
				}

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		// the move we will be returning
		size_t bestMoveIndex(0);

		// if we have an attack move we will use that one
		if (foundAction)
		{
			bestMoveIndex = actionMoveIndex;
		}
		// otherwise use the closest move to the opponent
		else
		{
			// if we are in attack range of the unit, back up
			if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
			{
				bestMoveIndex = furthestMoveIndex;
			}
			// otherwise get back into the fight
			else
			{
				bestMoveIndex = closestMoveIndex;
			}
		}
			
		move.addAction(moves.getMove(u, bestMoveIndex));
	}
}

PlayerPtr Player_Kiter::clone()
{
    return PlayerPtr(new Player_Kiter(*this));
}