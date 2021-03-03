// ChildRestriction chassis script
//
(function(child)
{
	let FamilyLM = 0x1100;
	let FamilyAIM = 0x1200;
	let FamilyAOM = 0x1300;
	let FamilyDIM = 0x1400;
	let FamilyDOM = 0x1500;
	let FamilyAIFM = 0x1600;
	let FamilyOCM = 0x1700;
	let FamilyWAIM = 0x1800;
	let FamilyTIM = 0x1900;
	let FamilyRIM = 0x1A00;
	let FamilyFIM = 0x1B00;
	
	let LMVersionPhase3 = 0;
	let LMVersionPhase4 = 1;
	
	let LMPlace = 0;
	let IOPlaceMin = 1;
	let IOPlaceMax = 14;

	if (child.isModule() == false)
	{ 
		return false; 
	}
	
	// Get module family, version and place
	//	
	let module = child.toModule();

	let family = module.moduleFamily;
	let version = module.moduleVersion;
	let place = module.place;
	
	// LM must be on place 0
	//
	if (family === FamilyLM)
	{
		if (place === LMPlace)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	// Input/Output modules should be on places IOPlaceMin..IOPlaceMax
	//
	if (place < IOPlaceMin || place > IOPlaceMax)
	{
		 return false; 
	} 
	
	// Determine LM version 
	//
	let lmVersion = -1;
	
	let chassis = child.parent();
	if (chassis == null || chassis.isChassis() === false)
	{
		return false;
	}
	
	let childrenCount = chassis.childrenCount;
	for (let i = 0; i < childrenCount; i++)
	{
		let childModule = chassis.child(i);
		if (childModule == null)
		{
			return false;
		}
		if (childModule.isModule() === false)
		{
			continue;
		}
		if (childModule.moduleFamily === FamilyLM)
		{
			lmVersion = childModule.moduleVersion;
			break;
		}
	}
	
	// Check compatibility for LMs
	
	if (lmVersion === LMVersionPhase3)
	{
		// LM1-SF00
		//
		if ((family === FamilyAIM && version === 0) ||	//AIM
			(family === FamilyAOM && version === 0) ||	//AOM
			(family === FamilyDIM && version === 0) ||	//DIM
			(family === FamilyDOM && version === 0) ||	//DOM
			(family === FamilyAIFM && version === 0) ||	//AIFM
			(family === FamilyOCM && version === 1))	//OCM
		{
			return true; 
		} 
		else
		{
			return false;
		}
	}
	if (lmVersion === LMVersionPhase4)
	{
		// LM1-SF40-4PH
		//
		if ((family === FamilyAIM && version === 1) ||	//AIM
			(family === FamilyAOM && version === 1) ||	//AOM
			(family === FamilyDIM && version === 0) ||	//DIM
			(family === FamilyDOM && version === 0) ||	//DOM
			(family === FamilyAIFM && version === 0) ||	//AIFM
			(family === FamilyOCM && version === 1) ||	//OCM
			(family === FamilyWAIM && version === 0) ||	//WAIM
			(family === FamilyTIM && version === 0) ||	//TIM
			(family === FamilyFIM && version === 0) ||	//FIM
			(family === FamilyRIM && version === 0))	//RIM
		{
			return true; 
		} 
		else
		{
			return false;
		}
	}
	
	return true;
})