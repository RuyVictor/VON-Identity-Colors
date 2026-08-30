//------------------------------------------------------------------------------------------------
// VON Identity Colors
// Configuration networking
//------------------------------------------------------------------------------------------------

modded class SCR_PlayerController
{
	//--------------------------------------------------------------------------------------------
	override protected void OnInit(
		IEntity owner
	)
	{
		super.OnInit(owner);


		if (!GetGame().InPlayMode())
			return;


		if (!Replication.IsServer())
			return;


		// Dedicated / listen server loads the authoritative JSON.
		VIC_ConfigService.EnsureServerLoaded();


		// Listen server / local host.
		//
		// Dedicated server simply has no local controller and exits
		// from VIC_ApplyLocalServerConfiguration().
		GetGame().GetCallqueue().CallLater(
			VIC_ApplyLocalServerConfiguration,
			250,
			false
		);
	}


	//--------------------------------------------------------------------------------------------
	override protected void OnOwnershipChanged(
		bool changing,
		bool becameOwner
	)
	{
		super.OnOwnershipChanged(
			changing,
			becameOwner
		);


		if (changing)
			return;


		if (!becameOwner)
			return;


		//----------------------------------------------------------------------------------------
		// Listen server
		//----------------------------------------------------------------------------------------

		if (Replication.IsServer())
		{
			GetGame().GetCallqueue().CallLater(
				VIC_ApplyLocalServerConfiguration,
				100,
				false
			);

			return;
		}


		//----------------------------------------------------------------------------------------
		// Remote client
		//----------------------------------------------------------------------------------------

		VIC_ConfigService.ResetClient();


		GetGame().GetCallqueue().CallLater(
			VIC_RequestConfiguration,
			250,
			false
		);


		// Safety retry.
		GetGame().GetCallqueue().CallLater(
			VIC_RequestConfigurationFallback,
			2000,
			false
		);
	}


	//--------------------------------------------------------------------------------------------
	protected void VIC_RequestConfiguration()
	{
		if (VIC_ConfigService.IsReady())
			return;


		Rpc(
			VIC_RPC_RequestConfiguration
		);
	}


	//--------------------------------------------------------------------------------------------
	protected void VIC_RequestConfigurationFallback()
	{
		// Reliable RPC normally makes this unnecessary.
		//
		// It is only a defensive retry for unusual ownership/init ordering.
		if (VIC_ConfigService.IsReady())
			return;


		Rpc(
			VIC_RPC_RequestConfiguration
		);
	}


	//--------------------------------------------------------------------------------------------
	// CLIENT -> SERVER
	//--------------------------------------------------------------------------------------------

	[RplRpc(
		RplChannel.Reliable,
		RplRcver.Server
	)]
	protected void VIC_RPC_RequestConfiguration()
	{
		string payload =
			VIC_ConfigService.BuildServerPayload();


		Rpc(
			VIC_RPC_ReceiveConfiguration,
			payload
		);
	}


	//--------------------------------------------------------------------------------------------
	// SERVER -> OWNER
	//--------------------------------------------------------------------------------------------

	[RplRpc(
		RplChannel.Reliable,
		RplRcver.Owner
	)]
	protected void VIC_RPC_ReceiveConfiguration(
		string payload
	)
	{
		VIC_ConfigService.ApplyNetworkPayload(
			payload
		);
	}


	//--------------------------------------------------------------------------------------------
	// Listen server local player
	//--------------------------------------------------------------------------------------------

	protected void VIC_ApplyLocalServerConfiguration()
	{
		PlayerController localController =
			GetGame().GetPlayerController();


		if (!localController)
			return;


		if (localController != this)
			return;


		string payload =
			VIC_ConfigService.BuildServerPayload();


		VIC_ConfigService.ApplyNetworkPayload(
			payload
		);
	}
}