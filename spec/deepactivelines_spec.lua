-- luacheck: std +busted
local deepactivelines = require "cluacov.deepactivelines"
local load = loadstring or load -- luacheck: compat

local sample = [[
local a = 1234
print(a)

local t = {
   [5] = 6
}

local function f()
   function t.a()
      local foo = "bar"
      return foo .. global
   end

   function t.b()
      return something
   end

   return x
end

function t.func()
   function t.c()
      return f()
   end

   function t.d()
      return t
   end
end

f()
return
]]

-- All active lines of the sample function should be within one
-- of these ranges, and each range should contain at least one active line.
local active_ranges = {
   {1, 2},
   {4, 6},
   {8, 12},
   {14, 16},
   {18, 19},
   {21, 24},
   {26, 29},
   {31, 32}
}

describe("deepactivelines", function()
   describe("version", function()
      it("is a string in MAJOR.MINOR.PATCH format", function()
         assert.match("^%d+%.%d+%.%d+$", deepactivelines.version)
      end)
   end)

   describe("get", function()
      it("throws error if the argument is not a function", function()
         assert.error(function() deepactivelines.get(5) end)
      end)

      it("throws error if the argument is a C function", function()
         assert.error(function() deepactivelines.get(deepactivelines.get) end)
      end)

      it("returns a table given an empty function", function()
         assert.table(deepactivelines.get(function() end))
      end)

      it("returns a set of line number given a function", function()
         local lines = deepactivelines.get(load(sample))
         assert.table(lines)
         local hit_ranges = {}

         for line, value in pairs(lines) do
            assert.number(line)
            assert.equal(true, value)
            local hit_range

            for _, range in ipairs(active_ranges) do
               if line >= range[1] and line <= range[2] then
                  hit_range = range
               end
            end

            assert.table(hit_range, (
               "Active line %d outside all ranges"):format(line))
            hit_ranges[hit_range] = true
         end

         for _, range in ipairs(active_ranges) do
            assert.equal(true, hit_ranges[range], (
               "Range %d..%d not hit"):format(range[1], range[2]))
         end
      end)

      it("returns an empty set given a stripped function", function()
         local stripped = load(string.dump(load(sample), true))
         
         if next(debug.getinfo(stripped, "L").activelines) then
            pending("string.dump can not strip functions")
         end

         assert.same({}, deepactivelines.get(stripped))
      end)
   end)
end)
