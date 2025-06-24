Dice = require'DICE'


function loadJson(filename)
  local f = assert(io.open(filename, "r"))
  local body = f:read("*all")
  f:close()
  return Json:Deserialize(body)
end

function gotoSystem(system)
  dofile(system.."/SYSTEM.LUA")
  Screen:push(system.."/SYSTEM")
  return true
end
