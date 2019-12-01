
------------------------------------------------
-- String utilities

function strok(s)
    return (type(s) == 'string' and s ~= '' and s)
end

function strnot(s)
    return (type(s) ~= 'string' or s == '')
end

function firstchar(s)
    return s and s:sub(1, 1)
end

function lastchar(s)
    return s and s:sub(-1)
end

function trim(s)
    -- trim both
    return s and s:match('^%s*(.-)%s*$')
end

function rtrim(s)
    -- trim right
    return s and s:match('(.-)%s*$')
end

function quote(s)
    return "'" .. tostring(s) .. "'"
end

function splitlines(s)
    -- split string of lines into table of each lines without CRLF
    lines = {}
    for ln in s:gmatch('([^\r\n]*)\r?\n') do
        table.insert(lines, ln)
    end
    return lines
end

function blankline(ln)
    return string.match(ln, '^%s*$')
end

function intok(n)
    -- is integer?, not float
    return (type(n) == 'number' and n == math.floor(n) and n)
end

function plusok(n)
    return (type(n) == 'number' and n > 0 and n)
end

function range ( from , to )
    return function (_,last)
            if last >= to then return nil
            else return last+1
            end
        end , nil , from-1
end

--[[
function ipairs_ordered(t)
    -- tmpIndex will hold sorted indices, otherwise
    -- this iterator would be no different from pairs iterator
    local tmpIndex = {}
    local index, _ = next(t)
    while index do
        tmpIndex[#tmpIndex+1] = index
        index, _ = next(t, index)
    end
    -- sort table indices
    table.sort(tmpIndex)
    local j = 1

    return function()
        -- get index value
        local i = tmpIndex[j]
        j = j + 1
        if i then
            return i, t[i]
        end
    end
end
]]

function ipairs_sorted(t, cmp)
    -- Enumearate sorted table. cmp is compare function to sort the table.
    -- If cmp is nil, enumerate by index order, same as ipairs_ordered()
    local tmpIndex = {}
    local index, assets = next(t)
    while index do
        tmpIndex[#tmpIndex+1] = index
        index, _ = next(t, index)
    end

    function compare ( ia, ib )
        return cmp(t[ia], t[ib])
    end

    -- sort table indices, using cmp as comparator
    table.sort(tmpIndex, cmp and compare or nil )
    local j = 1

    return function()
        -- get index value
        local i = tmpIndex[j]
        j = j + 1
        if i then
            return i, t[i]
        end
    end
end

---------------------------------------------
-- I/O lib Globals

EOFCHAR = '$'
EOSCHAR = '~'
EOSTR   = '~\n'
EOFENDING = '#99999\n$~'
EMPTY = ''

COMCHAR = '*'       -- at beginning of comment line

RENUMDIR = 'renum/'

KZone_datafile = nil
KZ_nextline = ''

Renum_outfile = nil
G_renum = false

--[[
-- this does not work for Windows
function pathexists(path)
    local fh = io.open(path, 'r')
    if fh then
        fh:close()
    end
    return fh
end
]]

function pathexists(path)
    local ok, err, code = os.rename(path, path)
    if not ok then
       if code == 13 then
          -- Permission denied, but it exists
          return true
       end
    end
    return ok, err
 end

function kz_openfile(fn, renumfn)
  -- file name is always relative to G_zonepath
  -- renum file name is relative to ./renum directory

    if strok(fn) then
      if KZone_datafile then
          KZone_datafile:close()
      end
      KZ_nextline = ''
      KZone_datafile = io.open( G_zonepath .. '/' .. fn)

      assert(KZone_datafile, "can't open " .. fn)
    end
    if G_renum and strok(renumfn) then
      if Renum_outfile then
          Renum_outfile:close()
      end
      Renum_outfile = io.open( RENUMDIR .. renumfn, 'w')
      assert(Renum_outfile)
    end
end

function kz_closefile()
    if KZone_datafile then
        KZone_datafile:close()
    end
    KZone_datafile = nil
    KZ_nextline = ''
    if Renum_outfile then
        Renum_outfile:close()
    end
    Renum_outfile = nil
end

function getline()
    -- Read single line without newline
    local ln

    if KZ_nextline ~= '' then
        ln = KZ_nextline
        KZ_nextline = ''
    else
        ln = KZone_datafile:read('*line')
    end

    return ln
end

function lookahead()
    if KZ_nextline == '' then
        KZ_nextline = getline()
    end
    return KZ_nextline
end

function pushback(s)
    assert(KZ_nextline == '', 'already pushed')
    KZ_nextline = s
end

function getzstring()
    -- Read text block terminated by '~\n' (removed)
    local zstr, ln, tr, ch
    zstr = ''
    while true do
        ln = getline()
        if not ln then return nil end
        tr = trim(ln)
        ch =  firstchar(tr)
        if ch == EOFCHAR or tr == 'S' or ch == '#' then
            pushback(ln)
            return nil
        end
        if lastchar(tr) == EOSCHAR then
            return zstr .. tr:sub(1,-2)
        end
        zstr = zstr .. ln .. '\n'
    end
end

function read_all_lines(fn)
    -- read all lines of file, return array of lines without CRLF
    local fd = io.open( G_zonepath .. '/' .. fn)
    local lines = splitlines(fd:read('*all'))
    fd:close()

    return lines
end


function nwrite(s)
    -- print without newline
    s = s or '<nil>'
    io.write(s)
end

function renum_writeln(s)
    if not G_renum then return end
    Renum_outfile:write(s)
    Renum_outfile:write('\n')
end

function renum_write(s)
    if not G_renum then return end
    s = s or '<nil>'
    Renum_outfile:write(s)
end

function EOFline(ln)
    return firstchar(ln) == EOFCHAR
end

function entry_id_to_num(ent_id)
    local nr = tonumber(string.match(ent_id, '#(%d+)'))
    assert(nr, 'not #id')
    return nr
end

-- =========================================================
----------------------
