#pragma once

typedef unsigned int UINT;

// Managed by the Content class. This will allow draw batching to be done much more cheaply.
struct Material
{
public:
	UINT
		textureID = -1,
		normalID = -1,
		specularID = -1,
		glossinessID = -1,
		ambientID = -1,
		heightID = -1,
		lightID = -1,
		occlusionID = -1,
		samplerID = -1,
		vsID = -1,
		psID = -1;

	Material() = default;
	Material(const Material &material) = default;

	bool operator==(const Material &other) const
	{
		return (textureID == other.textureID) &&
			(normalID == other.normalID) &&
			(specularID == other.specularID) &&
			(glossinessID == other.glossinessID) &&
			(ambientID == other.ambientID) &&
			(lightID == other.lightID) &&
			(occlusionID == other.occlusionID) &&
			(heightID == other.heightID) &&
			(samplerID == other.samplerID) &&
			(vsID == other.vsID) &&
			(psID == other.psID);
	}

	bool operator<(const Material &other) const
	{
		if (psID != other.psID)
			return psID < other.psID;

		if (vsID != other.vsID)
			return vsID < other.vsID;
		
		if (textureID != other.textureID)
			return textureID < other.textureID;

		if (normalID != other.normalID)
			return normalID < other.normalID;

		if (specularID != other.specularID)
			return specularID < other.specularID;

		if (ambientID != other.ambientID)
			return ambientID < other.ambientID;

		if (heightID != other.heightID)
			return heightID < other.heightID;

		if (occlusionID != other.occlusionID)
			return occlusionID < other.occlusionID;

		if (glossinessID != other.glossinessID)
			return glossinessID < other.glossinessID;
		
		if (lightID != other.lightID)
			return lightID < other.lightID;

		return samplerID < other.samplerID;
	}

	static inline Material MakeMat(
		UINT textureID = -1,	UINT normalID = -1, 
		UINT specularID = -1,	UINT glossinessID = -1,
		UINT ambientID = -1,    		
		UINT lightID = -1,      UINT occlusionID = -1,	
		UINT heightID = -1,		UINT samplerID = -1, 
		UINT vsID = -1,			UINT psID = -1)
	{
		Material mat;
		mat.textureID		= textureID;
		mat.normalID		= normalID;
		mat.specularID		= specularID;
		mat.glossinessID	= glossinessID;
		mat.ambientID		= ambientID;
		mat.lightID			= lightID;
		mat.occlusionID		= occlusionID;
		mat.heightID		= heightID;
		mat.samplerID		= samplerID;
		mat.vsID			= vsID;
		mat.psID			= psID;

		return mat;
	}
};