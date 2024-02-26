/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#include <engine/console.h>
#include <engine/storage.h>

#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>

#include "menus.h"
#include "countryflags.h"


void CCountryFlags::LoadCountryflagsIndexfile()
{
}

int CCountryFlags::GetInitAmount() const
{
	return 10;
}

void CCountryFlags::OnInit()
{
}

int CCountryFlags::Num() const
{
	return m_aCountryFlags.size();
}

const CCountryFlags::CCountryFlag *CCountryFlags::GetByCountryCode(int CountryCode) const
{
	return GetByIndex(m_CodeIndexLUT[maximum(0, (CountryCode-CODE_LB)%CODE_RANGE)]);
}

const CCountryFlags::CCountryFlag *CCountryFlags::GetByIndex(int Index, bool SkipBlocked) const
{
	return &m_aCountryFlags[maximum(0, Index%m_aCountryFlags.size())];
}

void CCountryFlags::Render(int CountryCode, const vec4 *pColor, float x, float y, float w, float h, bool AllowBlocked)
{
}
