//------------------------------------------------------------------------------------------------
// VON Identity Colors
//
// Identity (name) = WHO is talking.
// Communication UI (icon/glow/frequency) = WHERE they are talking.
//
// Default relationships:
//   Same group       = green
//   Same faction     = white
//   Allied faction   = cyan
//   Enemy            = red
//   Neutral          = purple
//   Unknown          = gray
//   Game Master      = magenta
//
// Default channels:
//   Direct           = white
//   Group            = green
//   Platoon/General  = orange
//   Objective        = blue
//   Commander        = gold
//   Custom           = gray
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
		// SCR_VonDisplay reuses receiving widget slots. Some vanilla code paths do not reset every
		// child color/visibility/opacity. Restore the properties VIC may have changed BEFORE vanilla
		// updates the new transmission. This fixes stale colors and hidden glow when a slot is reused.
		bool configReady = VIC_ConfigService.IsReady();

		if (
			IsReceiving
			&& configReady
			&& data
			&& !data.m_bIsAdditional
			&& data.m_Widgets
		)
		{
			VIC_RestoreVanillaVisualState(data);
		}

		// Vanilla always runs first and remains responsible for text, icon resources, visibility,
		// filtering, sender-editor state, role text, leader icon, etc.
		bool result = super.UpdateTransmission(
			data,
			radioTransceiver,
			frequency,
			IsReceiving
		);

		if (!result)
			return false;

		// Keep our own outgoing transmission completely vanilla. Vanilla uses special outgoing
		// colors/states (including muted radio), which VIC must not mask.
		if (!IsReceiving)
			return result;

		if (!data)
			return result;

		if (data.m_bIsAdditional)
			return result;

		if (!data.m_Widgets)
			return result;

		// Until authoritative server configuration arrives, leave VON untouched.
		if (!configReady)
			return result;

		VIC_Config config = VIC_ConfigService.GetConfig();
		if (!config)
			return result;

		// Because VIC restored its potentially stale state before calling vanilla, disabled VIC now
		// leaves a clean vanilla result instead of leaking a color from a previous slot occupant.
		if (!config.enabled)
			return result;

		bool isRadio = radioTransceiver != null;

		if (isRadio && !config.general.applyRadio)
			return result;

		if (!isRadio && !config.general.applyDirectSpeech)
			return result;

		//----------------------------------------------------------------------------------------
		// Resolve speaker relationship and channel.
		//----------------------------------------------------------------------------------------
		Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		Faction speakerFaction = data.m_Faction;

		if (!speakerFaction)
		{
			speakerFaction = SCR_FactionManager.SGetPlayerFaction(data.m_iPlayerID);
		}

		VIC_Relationship relationship = VIC_GetRelationship(
			data,
			localFaction,
			speakerFaction
		);

		VIC_ChannelType channel = VIC_GetChannelType(
			data,
			radioTransceiver,
			frequency,
			speakerFaction
		);

		Color relationshipColor = config.GetRelationshipColor(relationship);
		Color channelColor = config.GetChannelColor(channel);

		// Enemy/neutral/unknown can override a friendly-looking channel color. Game Master can also
		// override everything. This prevents, for example, an enemy transmission from looking orange
		// merely because its frequency numerically matches a platoon/general channel.
		if (
			relationship == VIC_Relationship.GAME_MASTER
			&& config.general.gameMasterOverridesAll
		)
		{
			channelColor = relationshipColor;
		}
		else if (
			config.general.nonFriendlyOverridesChannel
			&& VIC_IsNonFriendlyRelationship(relationship)
		)
		{
			channelColor = relationshipColor;
		}

		//----------------------------------------------------------------------------------------
		// NAME = relationship / identity
		//----------------------------------------------------------------------------------------
		if (
			config.elements.nameByRelationship
			&& data.m_Widgets.m_wName
		)
		{
			data.m_Widgets.m_wName.SetColor(relationshipColor);
		}

		//----------------------------------------------------------------------------------------
		// ICON = channel
		//----------------------------------------------------------------------------------------
		if (
			config.elements.iconByChannel
			&& data.m_Widgets.m_wIcon
		)
		{
			data.m_Widgets.m_wIcon.SetColor(channelColor);
		}

		//----------------------------------------------------------------------------------------
		// ICON GLOW
		//
		// IMPORTANT FIX:
		// Previously iconGlowByChannel=false only skipped SetColor(). Vanilla had already loaded
		// m_wIconBackground and VIC's reset colored it white, so the glow stayed visible.
		//
		// Now false explicitly hides the glow. True explicitly restores visibility, applies the
		// exact same final color used by the icon/channel, and attenuates it with iconGlowIntensity.
		//----------------------------------------------------------------------------------------
		if (data.m_Widgets.m_wIconBackground)
		{
			if (config.elements.iconGlowByChannel)
			{
				data.m_Widgets.m_wIconBackground.SetVisible(true);
				data.m_Widgets.m_wIconBackground.SetColor(channelColor);
				data.m_Widgets.m_wIconBackground.SetOpacity(config.elements.iconGlowIntensity);
			}
			else
			{
				data.m_Widgets.m_wIconBackground.SetOpacity(0.0);
				data.m_Widgets.m_wIconBackground.SetVisible(false);
			}
		}

		//----------------------------------------------------------------------------------------
		// RADIO FREQUENCY = channel
		//----------------------------------------------------------------------------------------
		if (
			isRadio
			&& config.elements.frequencyByChannel
			&& data.m_Widgets.m_wFrequency
		)
		{
			data.m_Widgets.m_wFrequency.SetColor(channelColor);
		}

		//----------------------------------------------------------------------------------------
		// SEPARATOR = channel
		//----------------------------------------------------------------------------------------
		if (
			isRadio
			&& config.elements.separatorByChannel
			&& data.m_Widgets.m_wSeparator
		)
		{
			data.m_Widgets.m_wSeparator.SetColor(channelColor);
		}

		//----------------------------------------------------------------------------------------
		// GAME MASTER BADGE
		//----------------------------------------------------------------------------------------
		if (
			relationship == VIC_Relationship.GAME_MASTER
			&& config.elements.gameMasterBadgeByRelationship
			&& data.m_Widgets.m_wGameMaster
		)
		{
			data.m_Widgets.m_wGameMaster.SetColor(relationshipColor);
		}

		//----------------------------------------------------------------------------------------
		// PLAYER ROLE / FUNCTION TEXT
		//----------------------------------------------------------------------------------------
		if (data.m_Widgets.m_wRole)
		{
			if (config.elements.hidePlayerRole)
			{
				data.m_Widgets.m_wRole.SetVisible(false);
			}
			else if (config.elements.roleByRelationship)
			{
				data.m_Widgets.m_wRole.SetColor(relationshipColor);
			}
		}

		//----------------------------------------------------------------------------------------
		// OPTIONAL SQUAD LEADER ICON
		//----------------------------------------------------------------------------------------
		if (
			config.elements.squadLeaderIconByRelationship
			&& data.m_Widgets.m_wSquadLeaderIcon
		)
		{
			data.m_Widgets.m_wSquadLeaderIcon.SetColor(relationshipColor);
		}

		//----------------------------------------------------------------------------------------
		// OPTIONAL ENEMY NAME HIDING
		// Only hide; never force visible, because vanilla/another mod may intentionally hide it.
		//----------------------------------------------------------------------------------------
		if (
			relationship == VIC_Relationship.ENEMY
			&& config.general.hideEnemyNames
			&& data.m_Widgets.m_wName
		)
		{
			data.m_Widgets.m_wName.SetVisible(false);
		}

		if (config.general.debugLogging)
		{
			PrintFormat(
				"[VON Identity Colors] Player=%1 Relationship=%2 Channel=%3 Frequency=%4 Radio=%5 Glow=%6 GlowIntensity=%7",
				data.m_iPlayerID,
				relationship,
				channel,
				frequency,
				isRadio,
				config.elements.iconGlowByChannel,
				config.elements.iconGlowIntensity
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
		if (data.m_bIsSenderEditor)
			return VIC_Relationship.GAME_MASTER;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (groupsManager)
		{
			int localPlayerId = SCR_PlayerController.GetLocalPlayerId();

			if (localPlayerId > 0)
			{
				SCR_AIGroup localGroup = groupsManager.GetPlayerGroup(localPlayerId);
				SCR_AIGroup speakerGroup = groupsManager.GetPlayerGroup(data.m_iPlayerID);

				if (
					localGroup
					&& speakerGroup
					&& localGroup.GetGroupID() == speakerGroup.GetGroupID()
				)
				{
					return VIC_Relationship.SAME_GROUP;
				}
			}
		}

		if (!localFaction)
			return VIC_Relationship.UNKNOWN;

		if (!speakerFaction)
			return VIC_Relationship.UNKNOWN;

		if (localFaction == speakerFaction)
			return VIC_Relationship.SAME_FACTION;

		if (localFaction.IsFactionFriendly(speakerFaction))
			return VIC_Relationship.ALLIED_FACTION;

		if (localFaction.IsFactionEnemy(speakerFaction))
			return VIC_Relationship.ENEMY;

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
		if (!radioTransceiver)
			return VIC_ChannelType.DIRECT;

		if (frequency <= 0)
			return VIC_ChannelType.CUSTOM;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		// Commander must be checked before generic group frequency because the commander network
		// is represented by a group with SCR_EGroupRole.COMMANDER.
		int commanderFrequency = VIC_GetCommanderFrequency(speakerFaction, groupsManager);

		if (
			commanderFrequency > 0
			&& commanderFrequency == frequency
		)
		{
			return VIC_ChannelType.COMMANDER;
		}

		// Faction / general / platoon frequency.
		SCR_Faction scrSpeakerFaction = SCR_Faction.Cast(speakerFaction);

		if (scrSpeakerFaction)
		{
			int platoonFrequency = scrSpeakerFaction.GetFactionRadioFrequency();

			if (
				platoonFrequency > 0
				&& platoonFrequency == frequency
			)
			{
				return VIC_ChannelType.PLATOON;
			}
		}

		// Any playable group frequency of the speaker's faction.
		if (VIC_IsKnownGroupFrequency(speakerFaction, frequency, groupsManager))
			return VIC_ChannelType.GROUP;

		// Objective/task-assigned radio frequency.
		SCR_GroupTaskManagerComponent taskManager = SCR_GroupTaskManagerComponent.GetInstance();

		if (
			taskManager
			&& speakerFaction
			&& taskManager.IsEnabledAssigningFrequencies()
		)
		{
			SCR_Task task = taskManager.GetTaskByFrequency(speakerFaction, frequency);

			if (task)
				return VIC_ChannelType.OBJECTIVE;
		}

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

		array<SCR_AIGroup> groups = groupsManager.GetPlayableGroupsByFaction(faction);

		if (!groups)
			return false;

		foreach (SCR_AIGroup group : groups)
		{
			if (!group)
				continue;

			if (group.GetRadioFrequency() == frequency)
				return true;
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

		array<SCR_AIGroup> groups = groupsManager.GetPlayableGroupsByFaction(faction);

		if (!groups)
			return 0;

		foreach (SCR_AIGroup group : groups)
		{
			if (!group)
				continue;

			if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
				return group.GetRadioFrequency();
		}

		return 0;
	}

	//============================================================================================
	// NON-FRIENDLY PRIORITY
	//============================================================================================
	protected bool VIC_IsNonFriendlyRelationship(VIC_Relationship relationship)
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
	// RESTORE STATE VIC MAY HAVE CHANGED
	//
	// Called BEFORE vanilla UpdateTransmission. This is deliberate: it removes stale VIC state,
	// then lets vanilla apply its own current-transmission state on top. In particular, it restores
	// m_wIconBackground visibility/opacity after a previous speaker had glow disabled.
	//============================================================================================
	protected void VIC_RestoreVanillaVisualState(TransmissionData data)
	{
		if (!data)
			return;

		if (!data.m_Widgets)
			return;

		Color vanillaWhite = Color.FromSRGBA(255, 255, 255, 255);

		if (data.m_Widgets.m_wName)
			data.m_Widgets.m_wName.SetColor(vanillaWhite);

		if (data.m_Widgets.m_wIcon)
			data.m_Widgets.m_wIcon.SetColor(vanillaWhite);

		if (data.m_Widgets.m_wIconBackground)
		{
			data.m_Widgets.m_wIconBackground.SetVisible(true);
			data.m_Widgets.m_wIconBackground.SetOpacity(1.0);
			data.m_Widgets.m_wIconBackground.SetColor(vanillaWhite);
		}

		if (data.m_Widgets.m_wFrequency)
			data.m_Widgets.m_wFrequency.SetColor(vanillaWhite);

		if (data.m_Widgets.m_wSeparator)
			data.m_Widgets.m_wSeparator.SetColor(vanillaWhite);

		// VIC may optionally recolor these. Reset before vanilla so vanilla can still apply its own
		// commander/leadership styling afterwards.
		if (data.m_Widgets.m_wRole)
			data.m_Widgets.m_wRole.SetColor(vanillaWhite);

		if (data.m_Widgets.m_wSquadLeaderIcon)
			data.m_Widgets.m_wSquadLeaderIcon.SetColor(vanillaWhite);
	}
}
