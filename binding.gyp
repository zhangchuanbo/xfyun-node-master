{
    "targets": [
        {
            "target_name": "tts",
            "sources": ["lib/src/tts.cpp"],
            "cflags_cc!": [ "-fno-exceptions" ],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "ExceptionHandling": 1,
                    "AdditionalOptions": []
                }
            },
            'conditions': [
                ['OS=="linux"', {
                    "cflags!": [ "-g", "-Wall", "-Ilib/src/sdk/include","-fno-exceptions" ],
                    "libraries": ["/usr/local/lib/libmsc.so"],
                }],
                ['OS=="win"', {
                    'copies':[
                    {'destination':'build/Release',
                        'files':["lib/src/sdk/bin/msc_x64.dll","lib/src/sdk/bin/msc.dll","lib/src/sdk/libs/msc_x64.lib"],
                        },
                    ],
                    "include_dirs": [
                        "lib/src/sdk/libs",
                    ],
                    "cflags!": [ "-g", "-Wall", "-Ilib/src/sdk/libs","-fno-exceptions" ],
                }],
            ],
            "include_dirs": [
                "lib/src/sdk/include",
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
        }
    ]
}
