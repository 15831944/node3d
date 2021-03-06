#include "world.h"
#include "PlayerMe.h"
#include "Audio.h"
#include "RPGSkyUIGraph.h"
#include "RenderSystem.h"
#include "float.h"
#include "intersect.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// # (leo123) : i think here too need set patches in define
// ... : void CWorld::create(UCHAR uMapID) : must take count of World% in data and dont use this kinda of switch
// ... : Why double { in renderDamageInfo?

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CWorld::CWorld()
{
	m_CurMap = 0xFFFFFFFF;
	// ----
	// # Change the fog color.
	// ----
	Fog fog		= getFog();
	// ----
	fog.color	= 0x00000000;
	// ----
	setFog(fog);
	// ----
	//this->SetTerrain(& m_Terrain);
	// ----
	m_DamageTextRender.load("Data\\Fonts\\fzl2jw.ttf");
	m_DamageTextRender.setShadowBorder(1);
	m_DamageTextRender.setFontSize(20);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CWorld::~CWorld()
{
	// ----
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CRole * CWorld::getFocusRole()
{
	iRenderNode* pObj = NULL;
	// ----
	if(getFocusNodes().getChildObj().size() == 0)
	{
		// ----
		// # No focus objects.
		// ----
		pObj = NULL;
	}
	else
	{
		pObj = *getFocusNodes().getChildObj().begin();
		// ----
		/*if(pObj != NULL) 
		{
			if(pObj->getType() != MAP_ROLE)
			{
				// ----
				// # its not a role.
				// ----
				pObj = NULL;
			}
		}*/
		// ----
		// # Point of object is null.
	}
	// ----
	return (CRole*)pObj;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CRole* CWorld::getRole(ULONG uID)
{
	auto it = m_mapRole.find(uID);
	// ----
	if(it != m_mapRole.end())
	{
		return it->second;
	}
	// ----
	return NULL;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CRole* CWorld::pickRole(const Vec3D & vRayPos , const Vec3D & vRayDir)
{
	CRole * pRole	= NULL;
	// ----
	float fFocusMin = FLT_MAX;
	float fMin		= 0.00;
	float fMax		= 0.00;
	// ----
	FOR_IN(it,m_mapRole)
	{
		if(&CPlayerMe::getInstance()==it->second)
 		{
 			continue;
 		}
		// ----
		if(it->second->intersect(vRayPos , vRayDir, fMin, fMax) != NULL)
		{
			if(fFocusMin > fMin)
			{
				pRole		= it->second;
				// ----
				fFocusMin	= fMax;
			}
		}
	}
	// ----
	return pRole;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

iRenderNode* CWorld::pickProps(const Vec3D& vRayPos , const Vec3D& vRayDir)
{
	iRenderNode * pProp	= NULL;
	// ----
	float fFocusMin = FLT_MAX;
	float fMin		= 0.00;
	float fMax		= 0.00;
	// ----
	FOR_IN(it,m_RenderNodes)
	{
		if((*it)->getID()==6|| //Tree07
			(*it)->getID()==133|| //PoseBox01
			(*it)->getID()==145|| //Furniture06
			(*it)->getID()==146) //Furniture07
		{
			// ----
			if((*it)->intersect(vRayPos , vRayDir, fMin, fMax) != NULL)
			{
				// ----
				// Props In Balk!!!
				// ----
				unsigned char uCellAtt = CWorld::getInstance().getSceneData()->getCellAttribute((*it)->getPos().x,(*it)->getPos().z);
				if (uCellAtt&TERRAIN_ATT_TYPE_BALK)
				{
					continue;
				}
				// ----
				if(fFocusMin > fMin)
				{
					pProp		= *it;
					// ----
					fFocusMin	= fMax;
				}
			}
		}
	}
	// ----
	return pProp;
}

bool CWorld::addRole(CRole * pRole)
{
	if (!pRole)
	{
		return false;
	}
	// ----
	if (pRole->getLocalBBox().vMin.x == FLT_MAX)
	{
		BBox box(-0.5f,0.0f,-0.5f, 0.5f,2.0f,0.5f);
		pRole->setLocalBBox(box);
	}
	// ----
	pRole->updateWorldBBox();
	pRole->updateWorldMatrix();
	// ----
	if(m_mapRole.find(pRole->getRoleID()) == m_mapRole.end())
	{
		m_mapRole[pRole->getRoleID()] = pRole;
		return true;
	}
	// ----
	return false;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CWorld::delRole(ULONG uID)
{
	bool bReturn = false;
	// ----
	if(CPlayerMe::getInstance().getRoleID() != uID)
	{
		auto it = m_mapRole.find(uID);
		// ----
		if(it != m_mapRole.end())
		{
			removeRenderNode(it->second);
			// ----
			m_mapRole.erase(it);
			// ----
			bReturn = true;
		}
	}
	// ----
	return bReturn;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::create(UCHAR uMapID)
{
	if(uMapID != m_CurMap)
	{
		m_pData = NULL;
		m_pSceneData = NULL;
		m_OctreeRoot.clearNodes();
		m_RenderNodes.clear();
		m_FocusNode.clearChildren();
		this->clearChildren();
		// ----
		m_CurMap	= uMapID;
		// ----
		char szMapFilname[256];
		sprintf(szMapFilname,"Data\\World%d\\EncTerrain%d.obj", m_CurMap+1, m_CurMap+1);
		setFilename(szMapFilname);
		//load();
		//setup();
		// ----
		//GetAudio().LoadMusic("Data\\Music\\main_theme.mp3");
		//GetAudio().PlayMusic(true,20);
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::updateDamageInfo(double fTime, float fElapsedTime)
{
	for(int i = m_dequeDamageInfo.size() -1 ; i >= 0 ; i--)
	{
		DamageInfo& damageInfo	=  m_dequeDamageInfo[i];
		damageInfo.fTime		+= fElapsedTime;
		// ----
		if(damageInfo.fTime > 5.0f)
		{
			m_dequeDamageInfo.erase(m_dequeDamageInfo.begin() + i);
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::renderDamageInfo()const
{
	Node3DUIGraph::getInstance().initDrawText();
	// ----
	//for (auto it=m_mapRole.begin();it!=m_mapRole.end();it++)
	{
		//it->second->drawName();
	}
	// ----
	FOR_IN(it,m_dequeDamageInfo)
	{
		Pos2D pos(0,0);
		// ----
		CRenderSystem::getSingleton().world2Screen(it->vPos, pos);
		pos.x-=40;
		pos.y-=logf(1.0f+it->fTime)*200;
		// ----
		//RECT rc = {pos.x - 40, pos.y - 30, pos.x + 40, pos.y}; 
		// ----
		m_DamageTextRender.drawUBB(pos.x, pos.y,&it->ubb);
		// ----
		//RPGSkyUIGraph::getInstance().drawText(it->wcsInfo.c_str(), it->wcsInfo.length() ,rc, ALIGN_TYPE_CENTER);
		//m_DamageTextRender.drawText(it->wcsInfo.c_str(),rc, it->wcsInfo.length(), (UINT)ALIGN_TYPE_CENTER);
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::addDamageInfo(Vec3D vPos,const std::wstring & wcsInfo)
{
	DamageInfo damageInfo;
	damageInfo.vPos = vPos;
	damageInfo.wcsInfo = wcsInfo;
	damageInfo.fTime = 0.0f;
	m_DamageTextRender.buildUBB(&damageInfo.ubb,wcsInfo.c_str(),80,wcsInfo.length(),ALIGN_TYPE_CENTER);
	// ----
	m_dequeDamageInfo.push_back(damageInfo);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::updateRender(const CFrustum& frustum)
{
	if (m_bRefreshViewport || m_OldFrustum!=frustum)
	{
		CScene::updateRender(frustum);
		// ----
		if (CWorld::getInstance().getSceneData())
		{
			unsigned char uTileID = CWorld::getInstance().getSceneData()->getCellTileID(CPlayerMe::getInstance().getCellPosX(), CPlayerMe::getInstance().getCellPosY() ,0);
			// ----
			// # hide the house wall
			if (uTileID==4)
			{
				for(auto it = m_RenderNodes.begin(); it!=m_RenderNodes.end();)
				{
					auto temp = it++;
					if ((*temp)->getID()==125||(*temp)->getID()==126)
					{
						m_RenderNodes.erase(temp);
					}
				}
			}
		}
	}
	// ----
	for(auto it = m_mapRole.begin() ; it != m_mapRole.end() ; it++)
	{
		removeRenderNode(it->second);
		// ----
		CrossRet crossRet = frustum.CheckAABBVisible(it->second->getWorldBBox());
		// ----
		if(cross_exclude != crossRet)
		{
			m_RenderNodes.push_back(it->second);
		}
	}
}

void CWorld::frameMoveRole(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	FOR_IN(it,m_mapRole)
	{
		it->second->frameMoveRole(mWorld, fTime, fElapsedTime);
	}
}

void CWorld::frameMove(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	setup();
	// ----
	CScene::frameMove(mWorld, fTime, fElapsedTime);
	// ----
	// # Delete the old roles
	// ----
	for(auto it = m_mapRole.begin() ; it != m_mapRole.end() ; it++)
	{
		if((it->second->getActionState() == CRole::DEATH) ||
			((CPlayerMe::getInstance().getPos()-it->second->getPos()).length() > PLAYER_VIEW_RANGE))
		{
			CWorld::getInstance().delRole(it->second->getRoleID());
			// ----
			break;
		}
	}
	// ----
	m_DamageTextRender.OnFrameMove();
	updateDamageInfo(fTime, fElapsedTime);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CWorld::render(int nRenderType)const
{
	CScene::render(nRenderType);
	// ----
	//renderDamageInfo();
	//m_Messages.frameRender();
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------