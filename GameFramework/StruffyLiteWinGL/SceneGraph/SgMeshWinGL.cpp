
///////////////////////////////////////////////////////////////////////////////
/// SgMeshWin32GL.cpp

#include "GLee.h"
#include "BtMemory.h"
#include "BtString.h"
#include "SgNodeWinGL.h"
#include "RsMaterialWinGL.h"
#include "RsImplWinGL.h"
#include "RsTextureWinGL.h"
#include "RsCamera.h"
#include "RsSceneWinGL.h"
#include "RsVertexBufferWinGL.h"
#include "RsIndexBufferWinGL.h"
#include "SgMeshWinGL.h"
#include "SgMaterialsWinGL.h"
#include "SgBlendShapeImpl.h"
#include "RsShaderWinGL.h"

////////////////////////////////////////////////////////////////////////////////
// FixPointers

void SgMeshWinGL::FixPointers( BaArchive *pArchive, BtU8* pMemory )
{
	// Set the file data
	m_pFileData = (BaSgMeshFileData*) pMemory;
}

////////////////////////////////////////////////////////////////////////////////
// Render

//virtual
void SgMeshWinGL::Render()
{
	if( 1 )
	{
		const RsCamera &camera = RsRenderTargetWinGL::GetCurrent()->GetCamera();

		// Cache the camera frustum
		const RsFrustum& frustum = camera.GetFrustum();

		// Cache the position
		const MtVector3& v3Position = m_pNode->GetWorldTransform().GetTranslation();

		// Get the AABB
		const MtAABB& AABB = m_pFileData->m_AABB;

		// Get the center
		MtVector3 v3Center = AABB.Center();

		// Get the edge
		MtVector3 v3Edge = v3Center + MtVector3( 0, 0, AABB.Radius() );

		// Transform the center
		v3Center *= m_pNode->GetWorldTransform();

		// Transform the edge
		v3Edge *= m_pNode->GetWorldTransform();

		// Get the distance in case the world transform scaled
		MtVector3 v3Diff = v3Edge - v3Center;

		// Get the radius
		BtFloat radius = v3Diff.GetLength();

		// Are we inside or outside the frustum
		if( frustum.IsInside( v3Center, radius ) == BtTrue )
		{
			m_pNode->m_pMaterials->Render();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render

void SgMeshWinGL::Render( SgMeshRenderable *pRenderable )
{
	RsSceneWinGL* pScene = (RsSceneWinGL*)m_pNode->m_pFileData->m_pScene;

	// Cache each material block
	BaMaterialBlockFileData* pMaterialBlock = pRenderable->m_pMaterialBlock;

	// Cache the material
	RsMaterialWinGL* pMaterial = (RsMaterialWinGL*) pMaterialBlock->m_pMaterial;

	// Set the vertex buffer
	pScene->pVertexBuffer( pMaterial->GetVertexType() )->SetStream();

	// Set the index stream
	pScene->pIndexBuffer()->SetIndices();

	// Cache the effect
	RsShaderWinGL* pShader = (RsShaderWinGL*)pRenderable->m_pShader;

	// Render the blend shape
	if( m_pNode->pBlendShape() != BtNull )
	{
		SgBlendShapeImpl* pBlendShape = (SgBlendShapeImpl*)  m_pNode->pBlendShape();
		(void)pBlendShape;
	}

	// Cache the current render target
	RsRenderTarget* pRenderTarget = pRenderable->m_pRenderTarget;

	// Cache the camera
	const RsCamera &camera = pRenderTarget->GetCamera();

	const MtMatrix4 &m4ViewScreen = pRenderTarget->GetCamera().GetViewProjection();
	const MtMatrix4 &m4World      = m_pNode->m_pFileData->m_m4World;
	const MtMatrix4 &m4View	      = camera.GetView();
	MtMatrix4 m4WorldViewScreen   = m4World * m4ViewScreen;
	MtMatrix4 m4WorldView		  = m4World * m4View;

	// Cache the technique
	const BtChar* pTechnique = pMaterial->GetTechniqueName();

	if(strstr(pTechnique, "Projected"))
	{
		int a = 0;
		a++;
	}
	if( strstr( pTechnique, "RsShaderZLT" ) )
	{
		int a=0;
		a++;
	}

	// Set the technique
	pShader->SetTechnique( pTechnique );
	pShader->SetMatrix( RsHandles_WorldViewScreen, m4WorldViewScreen );
	pShader->SetMatrix( RsHandles_WorldViewInverseTranspose, m4WorldView.GetInverse().GetTranspose() );
	pShader->SetMatrix( RsHandles_ViewInverseTranspose, m4View.GetInverse().GetTranspose() );
	pShader->SetMatrix( RsHandles_World, m4World );
	pShader->SetMaterial(pMaterial);
	
	// Ensure we have a material
	BtAssert( pMaterial != BtNull );

	// Loop through the materials
	for( BtU32 nRenderBlock=pMaterialBlock->m_nStartRenderBlock; nRenderBlock<pMaterialBlock->m_nEndRenderBlock; nRenderBlock++ )
	{
		// Cache each render block
		BaRenderBlockFileData* pRenderBlock = pScene->pRenderBlock( nRenderBlock );

		// Loop through the materials
		for( BtU32 nPrimitiveBlock=pRenderBlock->m_nStartPrimitiveBlock; nPrimitiveBlock<pRenderBlock->m_nEndPrimitiveBlock; nPrimitiveBlock++ )
		{
			// Cache each primitive block
			RsIndexedPrimitiveWinGL* pPrimitiveBlock = pScene->pPrimitiveBlock( nPrimitiveBlock );

			// Draw the primitives using the effect
			pShader->Draw( pPrimitiveBlock );
		}
	}
}
