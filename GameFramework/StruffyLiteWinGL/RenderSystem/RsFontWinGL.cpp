////////////////////////////////////////////////////////////////////////////////
// RsFontWin32GL

// Includes
#include "GLee.h"
#include "RsUtil.h"
#include "RsFontWinGL.h"
#include "RsShaderWinGL.h"
#include "RsImplWinGL.h"
#include "RsTextureWinGL.h"
#include "BaFileData.h"
#include "BtString.h"
#include "MtVector2.h"
#include "glfw.h"

////////////////////////////////////////////////////////////////////////////////
// FixPointers

void RsFontWin32GL::FixPointers( BtU8 *pFileData, BaArchive *pArchive )
{
	// Set the file data
	m_pFileData = (LBaFontFileData*) pFileData;

	BtU32 texturePages = m_pFileData->m_nTexturePages;

	for( BtU32 iTexture=0; iTexture<texturePages; iTexture++ )
	{
		// Set the texture
		m_pTextures[iTexture] = (RsTextureWinGL*) pArchive->GetResource( m_pFileData->m_nTextures[iTexture] );
	}
	m_ratio = 1.50f;
}

////////////////////////////////////////////////////////////////////////////////
// SetNewLineRatio

void RsFontWin32GL::SetNewLineRatio( BtFloat ratio )
{
	m_ratio = ratio;
}

////////////////////////////////////////////////////////////////////////////////
// GetDimension

// virtual
MtVector2 RsFontWin32GL::GetDimension( const BtChar* szText )
{
	MtVector2 v2LastPosition = MtVector2( 0, 0 );

	// Set the start position
	MtVector2 v2Position = MtVector2( 0, 0 );

	// Cache the string length
	BtU32 length = BtStrLength(szText);

	// Loop through the string
	for( BtU32 nCharacterIndex=0; nCharacterIndex<length; nCharacterIndex++ )
	{
		// Cache each character
		BtUChar Character = szText[nCharacterIndex];

		if( Character == '\n' )
		{
			// Cache the font character
			LBaFontChar& fontChar = m_pFileData->m_characters['A'];	
			v2Position.x = 0;
			v2Position.y += fontChar.m_fHeight * 1.50f;
			continue;
		}

		// Cache the font character
		LBaFontChar& fontChar = m_pFileData->m_characters[Character];

		// Set the dimension from the width and height of the texture
		MtVector2 v2Dimension = MtVector2( fontChar.m_fWidth, fontChar.m_fHeight );

		// Increment the last position
		v2LastPosition.x = MtMax( v2LastPosition.x, v2Position.x + v2Dimension.x );
		v2LastPosition.y = MtMax( v2LastPosition.y, v2Position.y + v2Dimension.y );

		// Increment the position
		v2Position.x += fontChar.m_nXAdvance;
	}

	MtVector2 v2Dimension = v2LastPosition;

	return v2Dimension;
}

////////////////////////////////////////////////////////////////////////////////
// Render

// virtual
MtVector2 RsFontWin32GL::Render( const MtVector2& v2StartPosition,
							     const RsColour &colour,
							     const BtChar* szText,
							     BtU32 sortOrder )
{
	return Render( v2StartPosition, MtVector2( 1.0f, 1.0f ), colour, szText, sortOrder );
}

////////////////////////////////////////////////////////////////////////////////
// Render

// virtual
MtVector2 RsFontWin32GL::Render( const MtVector2& v2StartPosition,
							     const MtVector2& v2Scale,
							     const RsColour &colour,
							     const BtChar* szText,
							     BtU32 sortOrder )
{
	const BtU32 MaxFontLength = 256;

	MtVector2 v2LastPosition = MtVector2( 0, 0 );

	// Cache the impl
	RsImplWinGL *pImpl = (RsImplWinGL*)RsImpl::pInstance();

	// Cache the render target
	RsRenderTarget *pRenderTarget = RsRenderTarget::GetCurrent();

	// Cache the camera
	RsCamera camera = pRenderTarget->GetCamera();

	// Cache the display width and height
	BtFloat Width  = (BtFloat)camera.GetViewport().m_width;
	BtFloat Height = (BtFloat)camera.GetViewport().m_height;

	// Cache the display width and height
	BtFloat fScaleWidth  = 1.0f / Width;
	BtFloat fScaleHeight = 1.0f / Height;

	// Allocate vertex
	RsVertex3 *pStartVertex = pImpl->StartVertex();

	// Set the start vertex
	RsVertex3 *pVertex = pStartVertex;

	// Cache the texture
	RsTextureWinGL* pTexture = (RsTextureWinGL*) m_pTextures[ 0 ];

	// Set the start position
	MtVector2 v2Position = v2StartPosition;

	// Cache the string length
	BtU32 length = strlen(szText);

	// Cache the texture width and height
	BtFloat TextureWidth  = 1.0f / (BtFloat) pTexture->GetWidth();
	BtFloat TextureHeight = 1.0f / (BtFloat) pTexture->GetHeight();

	BtU32 currentVertex = 0;

	// Loop through the string
	for( BtU32 nCharacterIndex=0; nCharacterIndex<length; nCharacterIndex++ )
	{
		// Cache each character
		BtUChar Character = szText[nCharacterIndex];

		if( Character == '\n' )
		{
			// Cache the font character
			LBaFontChar& fontChar = m_pFileData->m_characters['A'];	
			v2Position.x = v2StartPosition.x;
			v2Position.y += fontChar.m_fHeight * 1.50f * v2Scale.y;
			continue;
		}

		// Cache the font character
		LBaFontChar& fontChar = m_pFileData->m_characters[Character];

		// Set the dimension from the width and height of the texture
		MtVector2 v2Dimension = MtVector2( fontChar.m_fWidth, fontChar.m_fHeight );

		// Calculate the positions
		BtFloat fX0 = v2Position.x;
		BtFloat fX1 = fX0 + ( v2Dimension.x * v2Scale.x );
		BtFloat fY0 = v2Position.y + ( fontChar.m_nYOffset * v2Scale.y );
		BtFloat fY1 = fY0 + ( v2Dimension.y * v2Scale.y );

		RsVertex3 *pQuad = pVertex;

		// Copy these into vertex
		pVertex->m_v3Position = MtVector3(fX0, fY0, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U0 , fontChar.m_V0 );
		++pVertex;

		pVertex->m_v3Position = MtVector3(fX0, fY1, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U0, fontChar.m_V1 + 1 );
		++pVertex;

		pVertex->m_v3Position = MtVector3(fX1, fY0, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U1 + 1, fontChar.m_V0  );
		++pVertex;

		pVertex->m_v3Position = MtVector3(fX0, fY1, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U0 , fontChar.m_V1 + 1 );
		++pVertex;

		pVertex->m_v3Position = MtVector3(fX1, fY1, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U1 + 1, fontChar.m_V1 + 1 );
		++pVertex;

		pVertex->m_v3Position = MtVector3(fX1, fY0, 0.1f );
		pVertex->m_v2UV = MtVector2( fontChar.m_U1 + 1, fontChar.m_V0 );
		++pVertex;

		// Scale the position to local screen space -1 to 1
		for( BtU32 i=0; i<6; i++ )
		{
			// Set the colour
			pQuad[ i ].m_colour = colour.asWord();

			// Scale the uvs
			pQuad[ i ].m_v2UV.x *= TextureWidth;
			pQuad[ i ].m_v2UV.y *= TextureHeight;

			// Flip the y
			pQuad[ i ].m_v3Position.y = Height - pQuad[ i ].m_v3Position.y;

			pQuad[ i ].m_v3Position.x -= 0.5f;
			pQuad[ i ].m_v3Position.y -= 0.5f;

			// Scale from 0..width to 0..1
			pQuad[ i ].m_v3Position.x *= fScaleWidth;
			pQuad[ i ].m_v3Position.y *= fScaleHeight;

			// Scale from 0..1 to 0..2
			pQuad[ i ].m_v3Position.x *= 2.0f;
			pQuad[ i ].m_v3Position.y *= 2.0f;

			// Translate from 0..2 to -1..1
			pQuad[ i ].m_v3Position.x -= 1.0f;
			pQuad[ i ].m_v3Position.y -= 1.0f;
		}

		// Render the 6 new vertex

		// Increment the last position
		v2LastPosition.x = MtMax( v2LastPosition.x, v2Position.x + v2Dimension.x );
		v2LastPosition.y = MtMax( v2LastPosition.y, v2Position.y + v2Dimension.y );

		// Increment the position
		v2Position.x += ( fontChar.m_nXAdvance * v2Scale.x );

		currentVertex += 6;
	}

	// Setup the primitive
	RsPrimitiveWinGL *pPrimitive = pImpl->AddPrimitive();
	pPrimitive->m_primitiveType = GL_TRIANGLES;
	pPrimitive->m_numVertex	    = currentVertex;
	pPrimitive->m_nStartVertex  = pImpl->GetCurrentVertex();

	// End the current vertex
	pImpl->EndVertex( currentVertex );

	// Make a new font renderable
	RsFontRenderable *pFontRenderable = pImpl->AddFont();
	pFontRenderable->m_pFont = this;
	pFontRenderable->m_pVertex = pStartVertex;
	pFontRenderable->m_primitive = pPrimitive;

	// Validate the shader
	BtAssert( pFontRenderable->m_pShader != BtNull );

	// Add the font to the renderable list
	RsRenderTargetWinGL *pCurrentRenderTarget = (RsRenderTargetWinGL*)RsRenderTarget::GetCurrent();
	pCurrentRenderTarget->Add( sortOrder, pFontRenderable );

	// Calculate the dimension
	MtVector2 v2Dimension = v2LastPosition - v2StartPosition;

	// Return the dimension
	return v2Dimension;
}

////////////////////////////////////////////////////////////////////////////////
// Render

void RsFontWin32GL::Render( RsFontRenderable *pRenderable )
{
	// Cache the current render target
	RsRenderTarget *pRenderTarget = pRenderable->m_pRenderTarget;

	// Set the shader
	RsShaderWinGL *pShader = (RsShaderWinGL*)pRenderable->m_pShader;

	RsPrimitiveWinGL* pPrimitives = (RsPrimitiveWinGL*) pRenderable->m_primitive;

	if( pPrimitives->m_primitiveType == 1 )
	{
		int a=0;
		a++;
	}

	// Cache the camera
	const RsCamera &camera = pRenderTarget->GetCamera();

	const MtMatrix4& m4Projection = camera.GetViewProjection();
	const MtMatrix4& m4World      = MtMatrix4::m_identity;
	const MtMatrix4& m4View	      = camera.GetView();
	MtMatrix4 m4WorldViewScreen   = m4World * m4Projection;

	MtMatrix4 m4WorldView = m4World * m4View;
	pShader->SetCamera( camera );
	pShader->SetTechnique( "RsShaderT2" );
	pShader->SetMatrix( RsHandles_WorldViewScreen, m4WorldViewScreen );

	for( BtU32 i=0; i<7; i++ )
	{
		glDisableVertexAttribArray( i );
	}

	// Cache the texture
	RsTextureWinGL* pTexture = (RsTextureWinGL*) m_pTextures[ 0 ];

	// Set the texture
	pTexture->SetTexture();

	// Set vertex arrays
	BtU32 stride = sizeof(RsVertex3);

	// Cache the vertex buffer
	BtU32 vertexBuffer = RsImplWinGL::GetVertexBuffer();

	// Get the size
	BtU32 dataSize = pPrimitives->m_numVertex * stride;

	// bind VBO in order to use
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer );
	
	GLenum error;
	error = glGetError();
	(void)error;

	// upload data to VBO
	glBufferData(GL_ARRAY_BUFFER, dataSize, pRenderable->m_pVertex, GL_DYNAMIC_DRAW);
	error = glGetError();
	(void)error;

	/*
	struct RsVertex3
	{
	MtVector3		m_v3Position;
	MtVector3		m_v3Normal;
	BtU32			m_colour;
	MtVector2		m_v2UV;
	MtVector2		m_v2UV2;
	};
	*/

	BtU32 offset = 0;
	
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, GL_BUFFER_OFFSET( offset ) );
	offset += sizeof( MtVector3 );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, GL_BUFFER_OFFSET( offset ) );
	offset += sizeof( MtVector3 );

	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, GL_BUFFER_OFFSET( offset ) );
	offset += sizeof( BtU32 );

	glEnableVertexAttribArray( 5);
	glVertexAttribPointer( 5, 2, GL_FLOAT, GL_FALSE, stride,  GL_BUFFER_OFFSET( offset ) );
	offset += sizeof( MtVector2 );

	// Draw the primitives
	pShader->Draw( pRenderable->m_primitive );

}
