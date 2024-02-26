/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/color.h>
#include <base/math.h>
#include <base/system.h>


#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>
#include <engine/shared/jsonwriter.h>

#include "menus.h"
#include "skins.h"


const char * const CSkins::ms_apSkinPartNames[NUM_SKINPARTS] = {"body", "marking", "decoration", "hands", "feet", "eyes"}; /* Localize("body","skins");Localize("marking","skins");Localize("decoration","skins");Localize("hands","skins");Localize("feet","skins");Localize("eyes","skins"); */
const char * const CSkins::ms_apColorComponents[NUM_COLOR_COMPONENTS] = {"hue", "sat", "lgt", "alp"};

char *CSkins::ms_apSkinVariables[NUM_SKINPARTS] = {0};
int *CSkins::ms_apUCCVariables[NUM_SKINPARTS] = {0};
int *CSkins::ms_apColorVariables[NUM_SKINPARTS] = {0};

const float MIN_EYE_BODY_COLOR_DIST = 80.f; // between body and eyes (LAB color space)

int CSkins::SkinPartScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

int CSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

int CSkins::GetInitAmount() const
{
	return NUM_SKINPARTS*5 + 8;
}

void CSkins::OnInit()
{
}

void CSkins::AddSkin(const char *pSkinName)
{
}

void CSkins::RemoveSkin(const CSkin *pSkin)
{
}

int CSkins::Num()
{
	return m_aSkins.size();
}

int CSkins::NumSkinPart(int Part)
{
	return m_aaSkinParts[Part].size();
}

const CSkins::CSkin *CSkins::Get(int Index)
{
	return &m_aSkins[maximum(0, Index%m_aSkins.size())];
}

int CSkins::Find(const char *pName, bool AllowSpecialSkin)
{
	for(int i = 0; i < m_aSkins.size(); i++)
	{
		if(str_comp(m_aSkins[i].m_aName, pName) == 0 && ((m_aSkins[i].m_Flags&SKINFLAG_SPECIAL) == 0 || AllowSpecialSkin))
			return i;
	}
	return -1;
}

const CSkins::CSkinPart *CSkins::GetSkinPart(int Part, int Index)
{
	int Size = m_aaSkinParts[Part].size();
	return &m_aaSkinParts[Part][maximum(0, Index%Size)];
}

int CSkins::FindSkinPart(int Part, const char *pName, bool AllowSpecialPart)
{
	for(int i = 0; i < m_aaSkinParts[Part].size(); i++)
	{
		if(str_comp(m_aaSkinParts[Part][i].m_aName, pName) == 0 && ((m_aaSkinParts[Part][i].m_Flags&SKINFLAG_SPECIAL) == 0 || AllowSpecialPart))
			return i;
	}
	return -1;
}

void CSkins::RandomizeSkin()
{
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		int Hue = random_int() % 255;
		int Sat = random_int() % 255;
		int Lgt = random_int() % 255;
		int Alp = 0;
		if (p == 1) // SKINPART_MARKING
			Alp = random_int() % 255;
		int ColorVariable = (Alp << 24) | (Hue << 16) | (Sat << 8) | Lgt;
		*CSkins::ms_apUCCVariables[p] = true;
		*CSkins::ms_apColorVariables[p] = ColorVariable;
	}

	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		const CSkins::CSkinPart *s = GetSkinPart(p, random_int() % NumSkinPart(p));
		while(s->m_Flags&CSkins::SKINFLAG_SPECIAL)
			s = GetSkinPart(p, random_int() % NumSkinPart(p));
		mem_copy(CSkins::ms_apSkinVariables[p], s->m_aName, MAX_SKIN_ARRAY_SIZE);
	}
}

vec3 CSkins::GetColorV3(int v) const
{
	float Dark = DARKEST_COLOR_LGT/255.0f;
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, Dark+(v&0xff)/255.0f*(1.0f-Dark)));
}

vec4 CSkins::GetColorV4(int v, bool UseAlpha) const
{
	vec3 r = GetColorV3(v);
	float Alpha = UseAlpha ? ((v>>24)&0xff)/255.0f : 1.0f;
	return vec4(r.r, r.g, r.b, Alpha);
}

int CSkins::GetTeamColor(int UseCustomColors, int PartColor, int Team, int Part) const
{
	static const int s_aTeamColors[3] = {0xC4C34E, 0x00FF6B, 0x9BFF6B};

	int TeamHue = (s_aTeamColors[Team+1]>>16)&0xff;
	int TeamSat = (s_aTeamColors[Team+1]>>8)&0xff;
	int TeamLgt = s_aTeamColors[Team+1]&0xff;
	int PartSat = (PartColor>>8)&0xff;
	int PartLgt = PartColor&0xff;

	if(!UseCustomColors)
	{
		PartSat = 255;
		PartLgt = 255;
	}

	int MinSat = 160;
	int MaxSat = 255;

	int h = TeamHue;
	int s = clamp(mix(TeamSat, PartSat, 0.2), MinSat, MaxSat);
	int l = clamp(mix(TeamLgt, PartLgt, 0.2), (int)DARKEST_COLOR_LGT, 200);

	int ColorVal = (h<<16) + (s<<8) + l;
	if(Part == SKINPART_MARKING) // keep alpha
		ColorVal += PartColor&0xff000000;

	return ColorVal;
}

bool CSkins::ValidateSkinParts(char *apPartNames[NUM_SKINPARTS], int *pUseCustomColors, int *pPartColors, int GameFlags) const
{
	// force standard (black) eyes on team skins
	if(GameFlags&GAMEFLAG_TEAMS)
	{
		// TODO: adjust eye color here as well?
		if(str_comp(apPartNames[SKINPART_EYES], "colorable") == 0 || str_comp(apPartNames[SKINPART_EYES], "negative") == 0)
		{
			str_copy(apPartNames[SKINPART_EYES], "standard", MAX_SKIN_ARRAY_SIZE);
			return false;
		}
	}
	else
	{
		const int BodyColor = pPartColors[SKINPART_BODY];
		const int EyeColor = pPartColors[SKINPART_EYES];

		vec3 BodyHsl(((BodyColor>>16)&0xff)/255.0f, ((BodyColor>>8)&0xff)/255.0f, (BodyColor&0xff)/255.0f);
		vec3 EyeHsl(((EyeColor>>16)&0xff)/255.0f, ((EyeColor>>8)&0xff)/255.0f, (EyeColor&0xff)/255.0f);

		if(!pUseCustomColors[SKINPART_BODY])
			BodyHsl = vec3(0, 0, 1);

		const vec3 BodyLab = RgbToLab(HslToRgb(BodyHsl));

		if(str_comp(apPartNames[SKINPART_EYES], "negative") == 0)
		{
			if(!pUseCustomColors[SKINPART_EYES])
				EyeHsl = vec3(0, 0, 1);

			vec3 OrgEyeHsl = EyeHsl;
			EyeHsl.l *= 0.925f;

			const vec3 EyeLab = RgbToLab(HslToRgb(EyeHsl));
			if(distance(BodyLab, EyeLab) < MIN_EYE_BODY_COLOR_DIST)
			{
				OrgEyeHsl.l = clamp(OrgEyeHsl.l - 0.22f, 0.f, 1.f);

				// white eye can't go to black because of our DARKEST_COLOR_LGT restriction, so switch to standard (black) eyes
				if(OrgEyeHsl.l < DARKEST_COLOR_LGT/255.f)
					str_copy(apPartNames[SKINPART_EYES], "standard", MAX_SKIN_ARRAY_SIZE); // black
				else
				{
					pUseCustomColors[SKINPART_EYES] = 1;
					pPartColors[SKINPART_EYES] = (int(OrgEyeHsl.h*255) << 16) | (int(OrgEyeHsl.s*255) << 8) | (int(OrgEyeHsl.l*255));
				}

				return false;
			}
		}
		else if(str_comp(apPartNames[SKINPART_EYES], "colorable") == 0)
		{
			if(!pUseCustomColors[SKINPART_EYES])
				EyeHsl = vec3(0, 0, 1);

			vec3 OrgEyeHsl = EyeHsl;
			EyeHsl.l = clamp(EyeHsl.l * 0.0823f, 0.f, 1.f);

			const vec3 EyeLab = RgbToLab(HslToRgb(EyeHsl));
			if(distance(BodyLab, EyeLab) < MIN_EYE_BODY_COLOR_DIST)
			{
				OrgEyeHsl.l -= 0.6f;
				OrgEyeHsl.l = clamp(OrgEyeHsl.l, 0.f, 1.f);

				pUseCustomColors[SKINPART_EYES] = 1;
				pPartColors[SKINPART_EYES] = (int(OrgEyeHsl.h*255) << 16) | (int(OrgEyeHsl.s*255) << 8) | (int(OrgEyeHsl.l*255));

				return false;
			}
		}
	}

	return true;
}

bool CSkins::SaveSkinfile(const char *pSaveSkinName)
{
	char aBuf[IO_MAX_PATH_LENGTH];
	str_format(aBuf, sizeof(aBuf), "skins/%s.json", pSaveSkinName);
	IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return false;

	CJsonWriter Writer(File);

	Writer.BeginObject();
	Writer.WriteAttribute("skin");
	Writer.BeginObject();
	for(int PartIndex = 0; PartIndex < NUM_SKINPARTS; PartIndex++)
	{
		if(!ms_apSkinVariables[PartIndex][0])
			continue;

		// part start
		Writer.WriteAttribute(ms_apSkinPartNames[PartIndex]);
		Writer.BeginObject();
		{
			Writer.WriteAttribute("filename");
			Writer.WriteStrValue(ms_apSkinVariables[PartIndex]);

			const bool CustomColors = *ms_apUCCVariables[PartIndex];
			Writer.WriteAttribute("custom_colors");
			Writer.WriteBoolValue(CustomColors);

			if(CustomColors)
			{
				for(int c = 0; c < NUM_COLOR_COMPONENTS-1; c++)
				{
					int Val = (*ms_apColorVariables[PartIndex] >> (2-c)*8) & 0xff;
					Writer.WriteAttribute(ms_apColorComponents[c]);
					Writer.WriteIntValue(Val);
				}
				if(PartIndex == SKINPART_MARKING)
				{
					int Val = (*ms_apColorVariables[PartIndex] >> 24) & 0xff;
					Writer.WriteAttribute(ms_apColorComponents[3]);
					Writer.WriteIntValue(Val);
				}
			}
		}
		Writer.EndObject();
	}
	Writer.EndObject();
	Writer.EndObject();

	// add new skin to the skin list
	AddSkin(pSaveSkinName);
	return true;
}
