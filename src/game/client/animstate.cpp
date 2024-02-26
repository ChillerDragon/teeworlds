/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include "animstate.h"

static void AnimSeqEval(CAnimSequence *pSeq, float Time, CAnimKeyframe *pFrame)
{
}

static void AnimAddKeyframe(CAnimKeyframe *pSeq, CAnimKeyframe *pAdded, float Amount)
{
}

static void AnimAdd(CAnimState *pState, CAnimState *pAdded, float Amount)
{
}


void CAnimState::Set(CAnimation *pAnim, float Time)
{
}

void CAnimState::Add(CAnimation *pAnim, float Time, float Amount)
{
}
