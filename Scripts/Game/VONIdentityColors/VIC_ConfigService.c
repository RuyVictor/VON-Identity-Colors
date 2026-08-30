//------------------------------------------------------------------------------------------------
// VON Identity Colors
// Server-authoritative JSON configuration
//------------------------------------------------------------------------------------------------

class VIC_ConfigService
{
	static const string CONFIG_DIRECTORY =
		"$profile:VONIdentityColors";

	static const string CONFIG_FILE =
		"$profile:VONIdentityColors/VONIdentityColors.json";


	protected static ref VIC_Config s_RuntimeConfig;

	protected static bool s_bServerLoaded;
	protected static bool s_bRuntimeReady;


	//--------------------------------------------------------------------------------------------
	static VIC_Config GetConfig()
	{
		if (!s_RuntimeConfig)
		{
			s_RuntimeConfig = new VIC_Config();
			s_RuntimeConfig.Validate();
		}

		return s_RuntimeConfig;
	}


	//--------------------------------------------------------------------------------------------
	static bool IsReady()
	{
		if (Replication.IsServer())
		{
			EnsureServerLoaded();

			return s_bRuntimeReady;
		}

		return s_bRuntimeReady;
	}


	//--------------------------------------------------------------------------------------------
	static void EnsureServerLoaded()
	{
		if (!Replication.IsServer())
			return;

		if (s_bServerLoaded)
			return;

		LoadServerConfig();
	}


	//--------------------------------------------------------------------------------------------
	static bool LoadServerConfig()
	{
		if (!Replication.IsServer())
			return false;


		FileIO.MakeDirectory(
			CONFIG_DIRECTORY
		);


		VIC_Config config =
			new VIC_Config();


		//----------------------------------------------------------------------------------------
		// FIRST START
		//----------------------------------------------------------------------------------------

		if (!FileIO.FileExists(CONFIG_FILE))
		{
			config.Validate();

			bool saved =
				config.PackToFile(
					CONFIG_FILE
				);


			if (saved)
			{
				Print(
					"[VON Identity Colors] Default configuration created: "
					+ CONFIG_FILE,
					LogLevel.NORMAL
				);
			}
			else
			{
				Print(
					"[VON Identity Colors] Could not create configuration: "
					+ CONFIG_FILE,
					LogLevel.ERROR
				);
			}


			s_RuntimeConfig = config;

			s_bServerLoaded = true;
			s_bRuntimeReady = true;

			return saved;
		}


		//----------------------------------------------------------------------------------------
		// EXISTING JSON
		//----------------------------------------------------------------------------------------

		bool loaded =
			config.LoadFromFile(
				CONFIG_FILE
			);


		if (!loaded)
		{
			Print(
				"[VON Identity Colors] JSON could not be loaded. "
				+ "Using defaults. The invalid file was NOT overwritten: "
				+ CONFIG_FILE,
				LogLevel.ERROR
			);


			config =
				new VIC_Config();

			config.Validate();


			s_RuntimeConfig = config;

			s_bServerLoaded = true;
			s_bRuntimeReady = true;

			return false;
		}


		config.Validate();


		s_RuntimeConfig = config;

		s_bServerLoaded = true;
		s_bRuntimeReady = true;


		Print(
			"[VON Identity Colors] Configuration loaded: "
			+ CONFIG_FILE,
			LogLevel.NORMAL
		);


		return true;
	}


	//--------------------------------------------------------------------------------------------
	// Available for a future admin reload command.
	//--------------------------------------------------------------------------------------------

	static bool ReloadServerConfig()
	{
		if (!Replication.IsServer())
			return false;


		s_bServerLoaded = false;
		s_bRuntimeReady = false;


		return LoadServerConfig();
	}


	//--------------------------------------------------------------------------------------------
	static string BuildServerPayload()
	{
		EnsureServerLoaded();


		VIC_Config config =
			GetConfig();


		config.Pack();


		return config.AsString();
	}


	//--------------------------------------------------------------------------------------------
	static void ApplyNetworkPayload(
		string payload
	)
	{
		if (payload == string.Empty)
		{
			Print(
				"[VON Identity Colors] Empty configuration payload.",
				LogLevel.ERROR
			);

			return;
		}


		VIC_Config config =
			new VIC_Config();


		config.ExpandFromRAW(
			payload
		);


		config.Validate();


		s_RuntimeConfig = config;
		s_bRuntimeReady = true;


		Print(
			"[VON Identity Colors] Server configuration synchronized.",
			LogLevel.NORMAL
		);
	}


	//--------------------------------------------------------------------------------------------
	static void ResetClient()
	{
		if (Replication.IsServer())
			return;


		s_RuntimeConfig =
			new VIC_Config();

		s_RuntimeConfig.Validate();

		s_bRuntimeReady = false;
	}
}