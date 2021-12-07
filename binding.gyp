{
    "targets": [
        {
            "target_name": "tts",
            "sources": ["lib/src/tts.cpp"],
            "cflags!": [ "-g", "-Wall", "-Ilib/src/sdk/include","-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "libraries": ["/usr/local/lib/libmsc.so"],
            "include_dirs": [
                "./lib/src/sdk/include",
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
        }
    ]
}
