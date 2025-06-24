{
  "template": "Standard",
  "tileholder": {
    "tiles": [
      %{
        local characters = FileSystem.findfiles(system.folder .. "/*.CHR")
        for i,chfile in ipairs(characters) do
          local ch = loadJson(chfile)
      }%
        { 
          "type": "tile",
          "x": %<(i-1) % 3>%,
          "y": %<(i-1) // 3 * 3>%,
          "width": 1,
          "height": 3,
          "onTouch": "system.loadCharacter(\"%<chfile>%\"); return true",
          "children": [
            {
              "type": "image",
              "x": 0,
              "y": 0,
              "width": 1,
              "height": 3,
              "path": "%<ch["image"]>%"
            },
            {
              "type": "label",
              "x": 0,
              "y": 0,
              "width": 1,
              "height": 3,
              "justification": "center",
              "alignment": "bottom",
              "text": "%<ch["shortname"]>%"
            },
            {
              "type": "label",
              "x": 0,
              "y": 0,
              "width": 1,
              "height": 3,
              "justification": "right",
              "alignment": "top",
              "text": "%<ch["level"]>%"
            }
          ]
        }
      %{if #characters > i then}%,%{end}%
      %{end}%
    ]
  }
}
