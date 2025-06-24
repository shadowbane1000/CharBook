{
    "template": "Standard",
    "tileholder": {
        "tiles": [
            %{for i,tile in ipairs(screen["tiles"]) do}%
                {
                    "type": "tile",
                    "x": 0,
                    "y": %<(i-1)*3>%,
                    "width": 3,
                    "height": 3,
                    "onTouch": "return gotoSystem(\"%<tile["system"]>%\")",
                    "children": [
                    {
                        "type": "image",
                        "x": 0,
                        "y": 0,
                        "width": 3,
                        "height": 2,
                        "path": "%<tile["system"]>%/SYSTEM.png"
                    },
                    {
                        "type": "label",
                        "x": 0,
                        "y": 2,
                        "width": 3,
                        "height": 1,
                        "justification": "center",
                        "alignment": "center",
                        "text": "%<tile["text"]>%"
                    }
                    ]
                }
            %{if #screen["tiles"] > i then}%,%{end}%
            %{end}%
        ]
    }
}

