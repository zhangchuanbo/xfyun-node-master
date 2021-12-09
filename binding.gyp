{
    "targets": [
        {
            "target_name": "tts",
            "sources": ["lib/src/tts.cpp"],
            "cflags_cc!": [ "-fno-exceptions" ],
            "cflags!": ["-fno-exceptions" ],
            'conditions': [
                ['OS=="linux"', {
                     "libraries": ["/usr/local/lib/libmsc.so"],
                     "include_dirs": [
                        "lib/src/sdk/include/linux",
                    ],
                }],
                ['OS=="win"', {
                    'copies':[
                    {'destination':'build/Release',
                        'files':["lib/src/sdk/bin/msc_x64.dll","lib/src/sdk/bin/msc.dll","lib/src/sdk/libs/msc_x64.lib","lib/src/sdk/libs/msc.lib"],
                        },
                    ],
                    "include_dirs": [
                        "lib/src/sdk/include/win",
                    ],
                    "msvs_settings": {
                        "VCCLCompilerTool": {
                            "ExceptionHandling": 1,
                            "AdditionalOptions": []
                        }
                    },
                }],
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
        }
    ]
}
