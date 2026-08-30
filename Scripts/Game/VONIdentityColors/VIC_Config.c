//------------------------------------------------------------------------------------------------
// VON Identity Colors
// JSON configuration
//------------------------------------------------------------------------------------------------


//================================================================================================
// COLOR
//================================================================================================

class VIC_JSONColor : JsonApiStruct
{
	int r;
	int g;
	int b;
	int a;


	//--------------------------------------------------------------------------------------------
	void VIC_JSONColor(
		int red = 255,
		int green = 255,
		int blue = 255,
		int alpha = 255
	)
	{
		r = red;
		g = green;
		b = blue;
		a = alpha;

		RegV("r");
		RegV("g");
		RegV("b");
		RegV("a");
	}


	//--------------------------------------------------------------------------------------------
	void Validate()
	{
		r = ClampChannel(r);
		g = ClampChannel(g);
		b = ClampChannel(b);
		a = ClampChannel(a);
	}


	//--------------------------------------------------------------------------------------------
	protected int ClampChannel(int value)
	{
		if (value < 0)
			return 0;

		if (value > 255)
			return 255;

		return value;
	}


	//--------------------------------------------------------------------------------------------
	Color ToColor()
	{
		return Color.FromSRGBA(
			r,
			g,
			b,
			a
		);
	}
}


//================================================================================================
// GENERAL
//================================================================================================

class VIC_GeneralConfig : JsonApiStruct
{
	// Enable modification of proximity/direct voice.
	bool applyDirectSpeech;

	// Enable modification of radio voice.
	bool applyRadio;

	// Enemy name is hidden completely.
	bool hideEnemyNames;

	// Enemy / neutral / unknown relationships override the channel color.
	//
	// Example:
	//
	// Enemy on a Platoon frequency:
	// true  -> RED icon
	// false -> ORANGE icon
	//
	bool nonFriendlyOverridesChannel;

	// Game Master relationship color overrides all channel colors.
	bool gameMasterOverridesAll;

	// Print relationship/channel detection to script log.
	bool debugLogging;


	//--------------------------------------------------------------------------------------------
	void VIC_GeneralConfig()
	{
		applyDirectSpeech = true;
		applyRadio = true;

		hideEnemyNames = false;

		nonFriendlyOverridesChannel = true;
		gameMasterOverridesAll = true;

		debugLogging = false;

		RegV("applyDirectSpeech");
		RegV("applyRadio");

		RegV("hideEnemyNames");

		RegV("nonFriendlyOverridesChannel");
		RegV("gameMasterOverridesAll");

		RegV("debugLogging");
	}
}


//================================================================================================
// ELEMENTS
//================================================================================================

class VIC_ElementsConfig : JsonApiStruct
{
	// Player name uses relationship color.
	bool nameByRelationship;

	// Hide the vanilla group role text shown with the player name.
	//
	// Example:
	// (Rifleman) PlayerName
	//
	// true:
	// PlayerName
	bool hidePlayerRole;

	// Communication channel colors.
	bool iconByChannel;
	bool iconGlowByChannel;
	bool frequencyByChannel;
	bool separatorByChannel;

	// Game Master badge.
	bool gameMasterBadgeByRelationship;

	// Optional relationship coloring of vanilla role/leader UI.
	bool roleByRelationship;
	bool squadLeaderIconByRelationship;


	//--------------------------------------------------------------------------------------------
	void VIC_ElementsConfig()
	{
		nameByRelationship = true;

		// Preserve vanilla role text by default.
		hidePlayerRole = false;

		iconByChannel = true;
		iconGlowByChannel = true;

		frequencyByChannel = true;
		separatorByChannel = true;

		gameMasterBadgeByRelationship = true;

		roleByRelationship = false;
		squadLeaderIconByRelationship = false;


		RegV("nameByRelationship");

		RegV("hidePlayerRole");

		RegV("iconByChannel");
		RegV("iconGlowByChannel");

		RegV("frequencyByChannel");
		RegV("separatorByChannel");

		RegV("gameMasterBadgeByRelationship");

		RegV("roleByRelationship");
		RegV("squadLeaderIconByRelationship");
	}
}


//================================================================================================
// RELATIONSHIP COLORS
//
// WHO is speaking?
//================================================================================================

class VIC_RelationshipColors : JsonApiStruct
{
	// My squad/group
	ref VIC_JSONColor sameGroup;

	// Same faction, but another squad/group
	ref VIC_JSONColor sameFaction;

	// Different faction that is friendly with ours
	ref VIC_JSONColor alliedFaction;

	ref VIC_JSONColor enemy;
	ref VIC_JSONColor neutral;
	ref VIC_JSONColor unknown;

	ref VIC_JSONColor gameMaster;


	//--------------------------------------------------------------------------------------------
	void VIC_RelationshipColors()
	{
		// GREEN
		// My squad.
		sameGroup = new VIC_JSONColor(
			90,
			210,
			125,
			255
		);

		// Same faction, another squad.
		sameFaction = new VIC_JSONColor(
			255,
			255,
			255,
			255
		);

		// CYAN
		// External allied faction.
		alliedFaction = new VIC_JSONColor(
			65,
			210,
			220,
			255
		);

		// RED
		enemy = new VIC_JSONColor(
			230,
			45,
			45,
			255
		);

		// PURPLE
		neutral = new VIC_JSONColor(
			175,
			105,
			230,
			255
		);

		// GRAY
		unknown = new VIC_JSONColor(
			165,
			165,
			165,
			255
		);

		// MAGENTA
		// Different from platoon/general orange.
		gameMaster = new VIC_JSONColor(
			235,
			80,
			180,
			255
		);


		RegV("sameGroup");
		RegV("sameFaction");
		RegV("alliedFaction");

		RegV("enemy");
		RegV("neutral");
		RegV("unknown");

		RegV("gameMaster");
	}


	//--------------------------------------------------------------------------------------------
	void Validate()
	{
		if (!sameGroup)
			sameGroup = new VIC_JSONColor(90, 210, 125, 255);

		if (!sameFaction)
			sameFaction = new VIC_JSONColor(255, 255, 255, 255);

		if (!alliedFaction)
			alliedFaction = new VIC_JSONColor(65, 210, 220, 255);

		if (!enemy)
			enemy = new VIC_JSONColor(230, 45, 45, 255);

		if (!neutral)
			neutral = new VIC_JSONColor(175, 105, 230, 255);

		if (!unknown)
			unknown = new VIC_JSONColor(165, 165, 165, 255);

		if (!gameMaster)
			gameMaster = new VIC_JSONColor(235, 80, 180, 255);


		sameGroup.Validate();
		sameFaction.Validate();
		alliedFaction.Validate();

		enemy.Validate();
		neutral.Validate();
		unknown.Validate();

		gameMaster.Validate();
	}


	//--------------------------------------------------------------------------------------------
	Color GetColor(VIC_Relationship relationship)
	{
		switch (relationship)
		{
			case VIC_Relationship.SAME_GROUP:
				return sameGroup.ToColor();
	
			case VIC_Relationship.SAME_FACTION:
				return sameFaction.ToColor();
	
			case VIC_Relationship.ALLIED_FACTION:
				return alliedFaction.ToColor();
	
			case VIC_Relationship.ENEMY:
				return enemy.ToColor();
	
			case VIC_Relationship.NEUTRAL:
				return neutral.ToColor();
	
			case VIC_Relationship.GAME_MASTER:
				return gameMaster.ToColor();
	
			case VIC_Relationship.UNKNOWN:
				return unknown.ToColor();
		}
	
		return unknown.ToColor();
	}
}


//================================================================================================
// CHANNEL COLORS
//
// WHERE is the player speaking?
//================================================================================================

class VIC_ChannelColors : JsonApiStruct
{
	// Proximity voice
	ref VIC_JSONColor direct;

	// Squad/group frequency
	ref VIC_JSONColor group;

	// Faction / general / platoon
	ref VIC_JSONColor platoon;

	// Task/objective frequency
	ref VIC_JSONColor objective;

	// Commander network
	ref VIC_JSONColor commander;

	// Manual/modded/unknown frequency
	ref VIC_JSONColor custom;


	//--------------------------------------------------------------------------------------------
	void VIC_ChannelColors()
	{
		// WHITE
		direct = new VIC_JSONColor(
			255,
			255,
			255,
			255
		);

		// GREEN
		group = new VIC_JSONColor(
			90,
			210,
			125,
			255
		);

		// ORANGE
		//
		// General / faction / platoon
		platoon = new VIC_JSONColor(
			245,
			145,
			45,
			255
		);

		// BLUE
		objective = new VIC_JSONColor(
			70,
			145,
			240,
			255
		);

		// GOLD
		commander = new VIC_JSONColor(
			235,
			195,
			65,
			255
		);

		// GRAY
		custom = new VIC_JSONColor(
			165,
			165,
			165,
			255
		);


		RegV("direct");

		RegV("group");
		RegV("platoon");

		RegV("objective");
		RegV("commander");

		RegV("custom");
	}


	//--------------------------------------------------------------------------------------------
	void Validate()
	{
		if (!direct)
			direct = new VIC_JSONColor(255, 255, 255, 255);

		if (!group)
			group = new VIC_JSONColor(90, 210, 125, 255);

		if (!platoon)
			platoon = new VIC_JSONColor(245, 145, 45, 255);

		if (!objective)
			objective = new VIC_JSONColor(70, 145, 240, 255);

		if (!commander)
			commander = new VIC_JSONColor(235, 195, 65, 255);

		if (!custom)
			custom = new VIC_JSONColor(165, 165, 165, 255);


		direct.Validate();

		group.Validate();
		platoon.Validate();

		objective.Validate();
		commander.Validate();

		custom.Validate();
	}


	//--------------------------------------------------------------------------------------------
	Color GetColor(VIC_ChannelType channel)
	{
		switch (channel)
		{
			case VIC_ChannelType.DIRECT:
				return direct.ToColor();
	
			case VIC_ChannelType.GROUP:
				return group.ToColor();
	
			case VIC_ChannelType.PLATOON:
				return platoon.ToColor();
	
			case VIC_ChannelType.OBJECTIVE:
				return objective.ToColor();
	
			case VIC_ChannelType.COMMANDER:
				return commander.ToColor();
	
			case VIC_ChannelType.CUSTOM:
				return custom.ToColor();
		}
	
		return custom.ToColor();
	}
}


//================================================================================================
// ROOT
//================================================================================================

class VIC_Config : JsonApiStruct
{
	static const int CURRENT_VERSION = 4;

	int version;

	bool enabled;

	ref VIC_GeneralConfig general;
	ref VIC_ElementsConfig elements;

	ref VIC_RelationshipColors relationshipColors;
	ref VIC_ChannelColors channelColors;


	//--------------------------------------------------------------------------------------------
	void VIC_Config()
	{
		version = CURRENT_VERSION;

		enabled = true;

		general = new VIC_GeneralConfig();
		elements = new VIC_ElementsConfig();

		relationshipColors = new VIC_RelationshipColors();
		channelColors = new VIC_ChannelColors();


		RegV("version");
		RegV("enabled");

		RegV("general");
		RegV("elements");

		RegV("relationshipColors");
		RegV("channelColors");
	}


	//--------------------------------------------------------------------------------------------
	void Validate()
	{
		if (version <= 0)
			version = CURRENT_VERSION;

		if (!general)
			general = new VIC_GeneralConfig();

		if (!elements)
			elements = new VIC_ElementsConfig();

		if (!relationshipColors)
			relationshipColors = new VIC_RelationshipColors();

		if (!channelColors)
			channelColors = new VIC_ChannelColors();


		relationshipColors.Validate();
		channelColors.Validate();

		version = CURRENT_VERSION;
	}


	//--------------------------------------------------------------------------------------------
	Color GetRelationshipColor(VIC_Relationship relationship)
	{
		return relationshipColors.GetColor(
			relationship
		);
	}


	//--------------------------------------------------------------------------------------------
	Color GetChannelColor(VIC_ChannelType channel)
	{
		return channelColors.GetColor(
			channel
		);
	}
}