## CoF Manager

Cry of Fear addons manager.

## What is this?

CoF Manager is an addon for Cry of Fear, which allows you to expand the capabilities of the mod using third-party libraries.

## How to use?

To install CoF Manager, copy the **CrashRpt.dll** file to the **cryoffear/cl_dlls** folder.
To install mods - copy the mod to the **cryoffear/addons** folder.

## How is works?

For implementation in the game CoF Manager uses a feature of the client library, which loads the special library at the start of the game - **CrashRpt.dll**. This library is likely to be used for debugging crashes using vector exceptions, but we can use it as a mod loader.

After start, CoF Manager loads all the libraries that are in the **cryoffear/addons** folder. If the library was successfully loaded, then CoF Manager calls the exported functions of the loaded mod to initialize it:

* Init - a function in which entire mod initialization should take place. The arguments are the major and minor versions of the CoF Manager. The function returns bool - initialization result: true - initialization successful, false - initialization failed.
* PluginInfo - a function that gives information about the mod. Mod information is plugin_info_t structure, which contains the name of the mod, the name of the developer, and version information.
* GetGameVars - a function in which CoF Manager sends the engine and mod data, which the addon can use to work with the mod.

The CoF Manager API is quite raw, most likely in the future it will be radically redone.

## VAC?

I can not be sure about the possibility of being banned by the VAC system, however, during the entire testing and development of CoF Manager, I did not receive a single ban. Moreover, there is not a single mention of VAC blocking in Cry of Fear on the Internet.

If you in doubt, you can use the No-Steam version of the game.

## How to compile?

Use Visual Studio 2019 to compile the project.