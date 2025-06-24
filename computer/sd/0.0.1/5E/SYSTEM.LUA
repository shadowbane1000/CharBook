local P = {}
system = P

P.folder = "5E"

function P.release()
  _G['character'] = nil
  _G['system'] = nil
end

function P.loadCharacter(chfile)
  P.chfile = chfile
  local ch = loadJson(chfile)
  if not ch then
    return nil, "Character file not found: " .. chfile
  end
  global.character = ch
  Screen:push("5E/5ECHAR")
  return true
end

function P.saveCharacter()
  local f = assert(io.open(P.chfile, "w"))
  f:write(Json:Serialize(character))
  f:close()
end

function P.shortRest()
  P.saveCharacter()
end

function P.longRest()
  character["hp"]["current"] = character["hp"]["max"]
  character["hp"]["maxAdjustment"] = 0

  -- Reset spell slots, abilities, etc. as needed
  --for _, spell in ipairs(character["spells"]) do
  --  spell["used"] = 0
  --end

  P.saveCharacter()
end

return system

