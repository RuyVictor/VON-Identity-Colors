//------------------------------------------------------------------------------------------------
// VON Identity Colors
//
// NAME
//     Represents WHO is talking.
//
// ICON / GLOW / FREQUENCY
//     Represents WHERE they are talking.
//
// Examples:
//
// My squad + Group Radio
//     Name = GREEN
//     Icon = GREEN
//
// My squad + Platoon/General
//     Name = GREEN
//     Icon = ORANGE
//
// Same faction, different squad + General
//     Name = ORANGE
//     Icon = ORANGE
//
// Same faction, different squad + Direct
//     Name = ORANGE
//     Icon = WHITE
//
// Allied external faction + General
//     Name = CYAN
//     Icon = ORANGE
//
// Enemy
//     Name = RED
//     Icon = RED
//
//------------------------------------------------------------------------------------------------

modded class SCR_VonDisplay
{
	//--------------------------------------------------------------------------------------------
	override protected bool UpdateTransmission(
		TransmissionData data,
		BaseTransceiver radioTransceiver,
		int frequency,
		bool IsReceiving
	)
	{
		//----------------------------------------------------------------------------------------
		// VANILLA FIRST
		//----------------------------------------------------------------------------------------

		bool result =
			super.UpdateTransmission(
				data,
				radioTransceiver,
				frequency,
				IsReceiving
			);


		if (!result)
			return false;


		//----------------------------------------------------------------------------------------
		// OUTGOING
		//
		// Keep our own voice HUD completely vanilla.
		//
		// Vanilla uses special colors to represent transmission state,
		// including muted radio.
		//----------------------------------------------------------------------------------------

		if (!IsReceiving)
			return result;


		if (!data)
			return result;


		if (data.m_bIsAdditional)
			return result;


		if (!data.m_Widgets)
			return result;


		//----------------------------------------------------------------------------------------
		// Do nothing until the authoritative server JSON is available.
		//----------------------------------------------------------------------------------------

		if (!VIC_ConfigService.IsReady())
			return result;


		VIC_Config config =
			VIC_ConfigService.GetConfig();


		if (!config)
			return result;


		if (!config.enabled)
			return result;


		//----------------------------------------------------------------------------------------
		// RADIO / DIRECT FILTER
		//----------------------------------------------------------------------------------------

		bool isRadio =
			radioTransceiver != null;


		if (
			isRadio
			&& !config.general.applyRadio
		)
		{
			return result;
		}


		if (
			!isRadio
			&& !config.general.applyDirectSpeech
		)
		{
			return result;
		}


		//----------------------------------------------------------------------------------------
		// Reset only when we're actually going to manage this entry.
		//
		// This is intentionally AFTER enabled/channel checks so disabling
		// VIC doesn't unnecessarily overwrite another HUD mod.
		//
		// SCR_VonDisplay reuses receiving slots.
		//----------------------------------------------------------------------------------------

		VIC_ResetManagedColors(
			data
		);


		//----------------------------------------------------------------------------------------
		// FACTIONS
		//----------------------------------------------------------------------------------------

		Faction localFaction =
			SCR_FactionManager.SGetLocalPlayerFaction();


		Faction speakerFaction =
			data.m_Faction;


		if (!speakerFaction)
		{
			speakerFaction =
				SCR_FactionManager.SGetPlayerFaction(
					data.m_iPlayerID
				);
		}


		//----------------------------------------------------------------------------------------
		// WHO?
		//----------------------------------------------------------------------------------------

		VIC_Relationship relationship =
			VIC_GetRelationship(
				data,
				localFaction,
				speakerFaction
			);


		//----------------------------------------------------------------------------------------
		// WHERE?
		//----------------------------------------------------------------------------------------

		VIC_ChannelType channel =
			VIC_GetChannelType(
				data,
				radioTransceiver,
				frequency,
				speakerFaction
			);


		Color relationshipColor =
			config.GetRelationshipColor(
				relationship
			);


		Color channelColor =
			config.GetChannelColor(
				channel
			);


		//----------------------------------------------------------------------------------------
		// PRIORITIES
		//----------------------------------------------------------------------------------------

		if (
			relationship == VIC_Relationship.GAME_MASTER
			&& config.general.gameMasterOverridesAll
		)
		{
			channelColor =
				relationshipColor;
		}
		else if (
			config.general.nonFriendlyOverridesChannel
			&& VIC_IsNonFriendlyRelationship(
				relationship
			)
		)
		{
			// Example:
			//
			// Enemy transmitted on a frequency that numerically matches
			// a known platoon frequency.
			//
			// Don't make the enemy icon orange.
			channelColor =
				relationshipColor;
		}


		//========================================================================================
		// NAME = WHO?
		//========================================================================================

		if (
			config.elements.nameByRelationship
			&& data.m_Widgets.m_wName
		)
		{
			data.m_Widgets.m_wName.SetColor(
				relationshipColor
			);
		}


		//========================================================================================
		// ICON = WHERE?
		//========================================================================================

		if (
			config.elements.iconByChannel
			&& data.m_Widgets.m_wIcon
		)
		{
			data.m_Widgets.m_wIcon.SetColor(
				channelColor
			);
		}


		//========================================================================================
		// ICON GLOW
		//========================================================================================

		if (
			config.elements.iconGlowByChannel
			&& data.m_Widgets.m_wIconBackground
		)
		{
			data.m_Widgets.m_wIconBackground.SetColor(
				channelColor
			);
		}


		//========================================================================================
		// FREQUENCY
		//========================================================================================

		if (
			isRadio
			&& config.elements.frequencyByChannel
			&& data.m_Widgets.m_wFrequency
		)
		{
			data.m_Widgets.m_wFrequency.SetColor(
				channelColor
			);
		}


		//========================================================================================
		// SEPARATOR
		//========================================================================================

		if (
			isRadio
			&& config.elements.separatorByChannel
			&& data.m_Widgets.m_wSeparator
		)
		{
			data.m_Widgets.m_wSeparator.SetColor(
				channelColor
			);
		}


		//========================================================================================
		// GAME MASTER BADGE
		//========================================================================================

		if (
			relationship == VIC_Relationship.GAME_MASTER
			&& config.elements.gameMasterBadgeByRelationship
			&& data.m_Widgets.m_wGameMaster
		)
		{
			data.m_Widgets.m_wGameMaster.SetColor(
				relationshipColor
			);
		}


		//========================================================================================
		// OPTIONAL ROLE
		//========================================================================================

		if (
			config.elements.roleByRelationship
			&& data.m_Widgets.m_wRole
		)
		{
			data.m_Widgets.m_wRole.SetColor(
				relationshipColor
			);
		}


		//========================================================================================
		// OPTIONAL SQUAD LEADER ICON
		//========================================================================================

		if (
			config.elements.squadLeaderIconByRelationship
			&& data.m_Widgets.m_wSquadLeaderIcon
		)
		{
			data.m_Widgets.m_wSquadLeaderIcon.SetColor(
				relationshipColor
			);
		}


		//========================================================================================
		// HIDE ENEMY NAME
		//
		// We only HIDE here.
		//
		// Never force the name visible because vanilla or another mod
		// may have intentionally hidden it.
		//========================================================================================

		if (
			relationship == VIC_Relationship.ENEMY
			&& config.general.hideEnemyNames
			&& data.m_Widgets.m_wName
		)
		{
			data.m_Widgets.m_wName.SetVisible(
				false
			);
		}


		//========================================================================================
		// DEBUG
		//========================================================================================

		if (config.general.debugLogging)
		{
			PrintFormat(
				"[VON Identity Colors] Player=%1 Relationship=%2 Channel=%3 Frequency=%4 Radio=%5",
				data.m_iPlayerID,
				relationship,
				channel,
				frequency,
				isRadio
			);
		}


		return result;
	}


	//============================================================================================
	// RELATIONSHIP
	//============================================================================================

	protected VIC_Relationship VIC_GetRelationship(
		TransmissionData data,
		Faction localFaction,
		Faction speakerFaction
	)
	{
		//----------------------------------------------------------------------------------------
		// GAME MASTER
		//----------------------------------------------------------------------------------------

		if (data.m_bIsSenderEditor)
			return VIC_Relationship.GAME_MASTER;


		//----------------------------------------------------------------------------------------
		// SAME GROUP
		//----------------------------------------------------------------------------------------

		SCR_GroupsManagerComponent groupsManager =
			SCR_GroupsManagerComponent.GetInstance();


		if (groupsManager)
		{
			int localPlayerId =
				SCR_PlayerController.GetLocalPlayerId();


			if (localPlayerId > 0)
			{
				SCR_AIGroup localGroup =
					groupsManager.GetPlayerGroup(
						localPlayerId
					);


				SCR_AIGroup speakerGroup =
					groupsManager.GetPlayerGroup(
						data.m_iPlayerID
					);


				if (
					localGroup
					&& speakerGroup
					&& localGroup.GetGroupID()
						== speakerGroup.GetGroupID()
				)
				{
					return VIC_Relationship.SAME_GROUP;
				}
			}
		}


		//----------------------------------------------------------------------------------------
		// UNKNOWN
		//----------------------------------------------------------------------------------------

		if (!localFaction)
			return VIC_Relationship.UNKNOWN;


		if (!speakerFaction)
			return VIC_Relationship.UNKNOWN;


		//----------------------------------------------------------------------------------------
		// SAME FACTION, OTHER GROUP
		//
		// THIS IS THE ORANGE CASE.
		//----------------------------------------------------------------------------------------

		if (localFaction == speakerFaction)
			return VIC_Relationship.SAME_FACTION;


		//----------------------------------------------------------------------------------------
		// EXTERNAL ALLIED FACTION
		//----------------------------------------------------------------------------------------

		if (
			localFaction.IsFactionFriendly(
				speakerFaction
			)
		)
		{
			return VIC_Relationship.ALLIED_FACTION;
		}


		//----------------------------------------------------------------------------------------
		// ENEMY
		//----------------------------------------------------------------------------------------

		if (
			localFaction.IsFactionEnemy(
				speakerFaction
			)
		)
		{
			return VIC_Relationship.ENEMY;
		}


		//----------------------------------------------------------------------------------------
		// NEUTRAL
		//----------------------------------------------------------------------------------------

		return VIC_Relationship.NEUTRAL;
	}


	//============================================================================================
	// CHANNEL
	//============================================================================================

	protected VIC_ChannelType VIC_GetChannelType(
		TransmissionData data,
		BaseTransceiver radioTransceiver,
		int frequency,
		Faction speakerFaction
	)
	{
		//----------------------------------------------------------------------------------------
		// DIRECT / PROXIMITY
		//----------------------------------------------------------------------------------------

		if (!radioTransceiver)
			return VIC_ChannelType.DIRECT;


		if (frequency <= 0)
			return VIC_ChannelType.CUSTOM;


		SCR_GroupsManagerComponent groupsManager =
			SCR_GroupsManagerComponent.GetInstance();


		//----------------------------------------------------------------------------------------
		// COMMANDER
		//
		// Must come before normal GROUP.
		//
		// Commander frequency is the radio frequency of the group whose role
		// is SCR_EGroupRole.COMMANDER.
		//----------------------------------------------------------------------------------------

		int commanderFrequency =
			VIC_GetCommanderFrequency(
				speakerFaction,
				groupsManager
			);


		if (
			commanderFrequency > 0
			&& commanderFrequency == frequency
		)
		{
			return VIC_ChannelType.COMMANDER;
		}


		//----------------------------------------------------------------------------------------
		// PLATOON / GENERAL / FACTION
		//----------------------------------------------------------------------------------------

		SCR_Faction scrSpeakerFaction =
			SCR_Faction.Cast(
				speakerFaction
			);


		if (scrSpeakerFaction)
		{
			int platoonFrequency =
				scrSpeakerFaction.GetFactionRadioFrequency();


			if (
				platoonFrequency > 0
				&& platoonFrequency == frequency
			)
			{
				return VIC_ChannelType.PLATOON;
			}
		}


		//----------------------------------------------------------------------------------------
		// GROUP FREQUENCY
		//
		// Check every known playable group of the speaker's faction instead
		// of checking only the speaker's current squad.
		//
		// This also correctly classifies manually tuned squad frequencies.
		//----------------------------------------------------------------------------------------

		if (
			VIC_IsKnownGroupFrequency(
				speakerFaction,
				frequency,
				groupsManager
			)
		)
		{
			return VIC_ChannelType.GROUP;
		}


		//----------------------------------------------------------------------------------------
		// OBJECTIVE / TASK
		//
		// This mirrors the way the vanilla VON menu resolves a known
		// task frequency.
		//----------------------------------------------------------------------------------------

		SCR_GroupTaskManagerComponent taskManager =
			SCR_GroupTaskManagerComponent.GetInstance();


		if (
			taskManager
			&& speakerFaction
			&& taskManager.IsEnabledAssigningFrequencies()
		)
		{
			SCR_Task task =
				taskManager.GetTaskByFrequency(
					speakerFaction,
					frequency
				);


			if (task)
				return VIC_ChannelType.OBJECTIVE;
		}


		//----------------------------------------------------------------------------------------
		// CUSTOM / MANUAL / MODDED
		//----------------------------------------------------------------------------------------

		return VIC_ChannelType.CUSTOM;
	}


	//============================================================================================
	// KNOWN GROUP FREQUENCY
	//============================================================================================

	protected bool VIC_IsKnownGroupFrequency(
		Faction faction,
		int frequency,
		SCR_GroupsManagerComponent groupsManager
	)
	{
		if (!faction)
			return false;


		if (!groupsManager)
			return false;


		array<SCR_AIGroup> groups =
			groupsManager.GetPlayableGroupsByFaction(
				faction
			);


		if (!groups)
			return false;


		foreach (SCR_AIGroup group : groups)
		{
			if (!group)
				continue;


			if (
				group.GetRadioFrequency()
				== frequency
			)
			{
				return true;
			}
		}


		return false;
	}


	//============================================================================================
	// COMMANDER FREQUENCY
	//============================================================================================

	protected int VIC_GetCommanderFrequency(
		Faction faction,
		SCR_GroupsManagerComponent groupsManager
	)
	{
		if (!faction)
			return 0;


		if (!groupsManager)
			return 0;


		array<SCR_AIGroup> groups =
			groupsManager.GetPlayableGroupsByFaction(
				faction
			);


		if (!groups)
			return 0;


		foreach (SCR_AIGroup group : groups)
		{
			if (!group)
				continue;


			if (
				group.GetGroupRole()
				== SCR_EGroupRole.COMMANDER
			)
			{
				return group.GetRadioFrequency();
			}
		}


		return 0;
	}


	//============================================================================================
	// NON-FRIENDLY
	//============================================================================================

	protected bool VIC_IsNonFriendlyRelationship(
		VIC_Relationship relationship
	)
	{
		switch (relationship)
		{
			case VIC_Relationship.ENEMY:
			case VIC_Relationship.NEUTRAL:
			case VIC_Relationship.UNKNOWN:
				return true;
		}


		return false;
	}


	//============================================================================================
	// RESET
	//============================================================================================

	protected void VIC_ResetManagedColors(
		TransmissionData data
	)
	{
		if (!data)
			return;


		if (!data.m_Widgets)
			return;


		Color vanillaWhite =
			Color.FromSRGBA(
				255,
				255,
				255,
				255
			);


		// Name
		if (data.m_Widgets.m_wName)
		{
			data.m_Widgets.m_wName.SetColor(
				vanillaWhite
			);
		}


		// VON icon
		if (data.m_Widgets.m_wIcon)
		{
			data.m_Widgets.m_wIcon.SetColor(
				vanillaWhite
			);
		}


		// Icon glow
		if (data.m_Widgets.m_wIconBackground)
		{
			data.m_Widgets.m_wIconBackground.SetColor(
				vanillaWhite
			);
		}


		// Frequency
		if (data.m_Widgets.m_wFrequency)
		{
			data.m_Widgets.m_wFrequency.SetColor(
				vanillaWhite
			);
		}


		// Frequency separator
		if (data.m_Widgets.m_wSeparator)
		{
			data.m_Widgets.m_wSeparator.SetColor(
				vanillaWhite
			);
		}
	}
}