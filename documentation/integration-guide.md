# Integration Guide

To run end-to-end tests on your application, you need to integrate the C++
library, as well as using the JavaScript package.

## C++ Library

### Option 1: Pre-built library with CMake

1. Download the latest archive(s) from [GitHub releases](https://github.com/Focusrite-Novation/juce-end-to-end/releases) 
and add to your repository
1. Add the following to your CMakeLists.txt (after JUCE has been made available)
, changing the paths to reflect the layout of your repository
    ```CMake
    include (FetchContent)

    if (APPLE)
        set (FOCUSRITE_E2E_ARCHIVE_PATH path/to/focusrite-e2e-macos.tar.gz)
    elseif (WIN32)
        set (FOCUSRITE_E2E_ARCHIVE_PATH path/to/focusrite-e2e-windows.tar.gz)
    endif ()

    fetchcontent_declare (focusrite-e2e URL ${FOCUSRITE_E2E_ARCHIVE_PATH})
    fetchcontent_getproperties (focusrite-e2e)
    if (NOT focusrite-e2e_POPULATED)
        fetchcontent_populate (focusrite-e2e)
        list (APPEND CMAKE_PREFIX_PATH ${focusrite-e2e_SOURCE_DIR}/lib/cmake)
    endif ()

    find_package (FocusriteE2E REQUIRED)
    ```
1. Link your target with the `focusrite-e2e::focusrite-e2e` target
    ```CMake
    target_link_libraries (MyApp PRIVATE focusrite-e2e::focusrite-e2e)

### Option 2: Pre-built library manually

1. Download the latest archive(s) from [GitHub releases](https://github.com/Focusrite-Novation/juce-end-to-end/releases)
, unpack, and add to your repository
1. Add the enclosed `include` folder to your include path
1. Link with the appropriate library in the `lib` folder
1. Ensure `juce/modules` is available on your include path
1. Ensure JUCE is linked with your executable

### Option 3: From source with CMake

1. Add [Focusrite-Novation/juce-end-to-end](https://github.com/Focusrite-Novation/juce-end-to-end) 
as a submodule to your project
1. In your CMakeLists.txt, add the subdirectory (after JUCE has been made 
available)
    ```CMake
    add_subdirectory (lib/juce-end-to-end)
    ```
1. Link your target with the `focusrite-e2e::focusrite-e2e` target
    ```CMake
    target_link_libraries (MyApp PRIVATE focusrite-e2e::focusrite-e2e)
    ```

### Option 4: From source manually

1. Add [Focusrite-Novation/juce-end-to-end](https://github.com/Focusrite-Novation/juce-end-to-end) 
as a submodule to your project
1. Add the enclosed `include` folder to your include path
1. Compile all sources in the `sources` folder
1. Ensure `juce/modules` is available on your include path
1. Ensure JUCE is linked with your executable

### Create a `TestCentre`

All you need to do to make your application testable is create a 
`focusrite::e2e::TestCentre` that has the same lifecycle as your application. 
For example:

```C++
#include <focusrite/e2e/TestCentre.h>

class Application : public JUCEApplication
{
public:

    void initialise (const juce::String &) override
    {
        testCentre = focusrite::e2e::TestCentre::create ();
    }

    void shutdown () override
    {
        testCentre.reset ();
    }

private:
    std::unique_ptr <focusrite::e2e::TestCentre> testCentre;
}
```

If you wish to reference a `Component` by ID, you can do so as follows:

```C++
myComponent.getProperties ().set ("test-id", "my-component-id");
```

If you wish to install a custom request handler in addition to the default one,
you can do so as follows:

```C++
#include <focusrite/e2e/CommandHandler>

class MyCommandHandler : public focusrite::e2e::CommandHandler
{
public:

    std::optional<focusrite::e2e::Response> process (const focusrite::e2e::Command & command) override
    {
        if (command.getType () == "my-custom-type-1")
            return generateResponse1 (command);
        else if (command.getType () == "my-custom-type-2")
            return generateResponse2 (command);

        return std::nullopt;
    }
};

MyCommandHandler commandHandler;
testCentre->addCommandHandler (commandHandler);
```

You can also send custom events at any time, without needing to wait for a 
request:

```C++
const auto event = focusrite::e2e::Event ("something-happened")
        .withParameter("count", 5));
testCentre->sendEvent (event);
```

## JavaScript library

The JavaScript library provides utilities to start the application, send it
commands and query its state. You can use it with any testing/assertion library
(e.g. Jest, Mocha). The examples below are written in TypeScript using Jest.

1. Add the following line to a `.npmrc` file at the root of your repository:
    ```
    @focusrite-novation:registry=https://npm.pkg.github.com
    ```
1. Install the library using npm
    ```
    npm install @focusrite-novation/juce-end-to-end
    ```
1. In your test setup, before each test, create an `AppConnection` (passing it 
the path to your built application) and wait for it to launch. You can pass the
app extra arguments or set environment variables if you need to put it into 
special state.

    ```TypeScript
    import {AppConnection} from '@focusrite-novation/juce-end-to-end';

    describe('My app tests', () => {
        let appConnection: AppConnection;

        beforeEach(async () => {
            appConnection = new AppConnection({appPath: 'path/to/app/binary'});
            await appConnection.launch();
        });
    }
    ```

1. After each test, quit the app and wait for it to shut down:

    ```TypeScript
    afterEach(async () => {
        await appConnection.quit();
    });
    ```

1. During each test, you can send the application commands to query the state
of the user interface. Here is a simple example:

    ```TypeScript
    it('Increments using the increment button', async () => {
        const valueBefore = await appConnection.getComponentText('value-label');
        expect(valueBefore).toEqual('0');

        await appConnection.clickComponent('increment-button');

        const valueAfter = await appConnection.getComponentText('value-label');
        expect(valueAfter).toEqual('1');
    });
    ```

See the documentation for `AppConnection` for the full range of supported 
commands and responses. If you need to extend this, you can send custom commands,
as long as it can be serialised to JSON using 
`appConnection.sendCommand (myCustomCommand);`