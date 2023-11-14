# Inworld.AI Unreal Engine SDK

The **Inworld.AI Unreal Engine SDK** enables Developers to integrate Inworld.ai characteres into Unreal Engine projects.

## Supported platforms

<table>
  <tr>
    <td><b>Windows</b></td>
  </tr>
  <tr>
    <td><b>Mac</b></td>
  </tr>
  <tr>
    <td><b>iOS</b></td>
  </tr>
  <tr>
    <td><b>Android</b></td>
  </tr>
</table>

## Getting started

- Download latest release to use Inworld.AI Unreal SDK on supported platforms.
- Clone this repository to get third party sources and build on other platforms. Update submodules recursively after clonning *git submodule update --init --recursive*. You might need to disable git path length limit on Windows in case *Filename too long* errors. 

### NDK Dependency
The Inworld.AI Unreal Engine SDK depends on the Inworld NDK.

#### Build the NDK
To build the NDK, utilize the script at InworldAI/Source/ThirdParty/Inworld/dev-tools. The arguments to the python script are as follows:

-c Clean the Build and Copy from NDK folders

-b -p 'platform' Build the NDK for 'platform' [Win64, iOS, Mac, Android]

-x Copy the NDK to the Unreal Module folder

Example: ```python ndk-util.py -c -b -p Win64 -x```

#### Download prebuilt NDK
For most use cases, simply downloading prebuilt versions of NDK will suffice.

Please find a copy of the Inworld.AI Unreal Engine SDK bundled with the latest NDK packages here: https://github.com/inworld-ai/inworld-unreal-sdk/releases/latest

### Additional Documentation
Please visit our [Unreal Engine page](https://docs.inworld.ai/docs/tutorial-integrations/unreal-engine/) for full documentation and setup instructions.
