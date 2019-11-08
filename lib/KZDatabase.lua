-- =========================================================

-- Zone lib Globals

ENTGPVAR = 8
REFCNT =  ENTGPVAR     -- used as ref counter
RENUMINDEX = 6

INDEX_OOB = 999999

-- { zone_name, zone_file, world_file, zone_base, zone_top, renum }
Zone_index = {}

--{ zona_name, zone_base, zone_renum_base }
Zone_renum_tab = {}

--  { rnr, rid, rinfo, rexits, rxtras, renum, 0, gpvar  }
Room_index = {}
Exit_index = {}

-- { onr, oid, oinfo, oname, oxtras, renum, 0, gpvar }
Object_index = {}
Mobile_index = {}

G_zone_number = -1
G_zone_name = ''

G_last_ztop = -1

G_room_min = INDEX_OOB
G_room_max = -1

function entry_idnum(entry)
    return entry and entry[1]
end

function entry_id(entry)
    return entry and entry[2]
end

function assets_name(entry)
    return entry and entry[4]
end

function zone_number(znr)
    if znr then
        G_zone_number = znr
    end
    return G_zone_number
end

function zone_name(iz)
    return Zone_index[ iz or G_zone_number][1]
end

function zone_base(iz)
    return Zone_index[ iz or G_zone_number][4]
end

function zone_top(iz)
    return Zone_index[ iz or G_zone_number][5]
end

function out_of_zone(idn, iz)
    return idn > 0 and (idn < zone_base(iz) or idn > zone_top(iz))
end

function find_zone(idn)

    for iz, zinfo in ipairs(Zone_index) do
        if idn >= zinfo[4] and idn <= zinfo[5] then     -- zone_base, zone_top
            return iz
        end
    end
    return nil
end

function renum_zone_offset(znum)
    return Zone_index[znum][6] - Zone_index[znum][4]
end

function load_zone_index()

    local zflines, wflines, last_ztop = -1

    zflines = read_all_lines('zone/zone_files')

    wflines = read_all_lines('world/world_files')

    for iz, zfn in ipairs(zflines) do

        local ln, zname, ztop, renum_zname

        if EOFline(zfn) then break end

        kz_openfile(zfn)

        zname = getzstring()
        ln =  getline()
        ztop = tonumber(ln:match('(%d+)%s'))

        renum_zname = Zone_renum_tab[iz][1]

        assert(zname == renum_zname, 'zone name mismatch: ' .. zname .. ', ' .. renum_zname )

        -- { zone_name, zone_file, world_file, zone_base, zone_top }
        Zone_index[iz] = { zname, zfn:gsub( 'zone/', ''),  wflines[iz]:gsub( 'world/', ''),
                Zone_renum_tab[iz][2], ztop, Zone_renum_tab[iz][3] }

        -- buf = string.format("    { '%s', '%s', '%s', %d, %d, %d, %d, %d, %d }",
        --     zname, zfn:gsub( 'zone/', ''), wflines[iz]:gsub( 'world/', ''),
        --     zbase, ztop, 0, 0, 0, 0 )

    end

end

function renum_zone_list()

    assert(G_renum)

    zf = io.open('renum/zone/zone_file', 'w')
    wf = io.open('renum/world/world_files', 'w')

    for iz, zinfo in ipairs_sorted(Zone_index, renum_cmp) do
        zf:write( 'zone/' .. zinfo[2] .. '\n');
        wf:write( 'world/' .. zinfo[3] .. '\n' );
    end
    zf:write('$\n') ; zf:close()
    wf:write('$\n') ; wf:close()
end


    --[[
function check_zone_list ()

    kz_openfile('zone/zone_files')

    local ln

    for iz, zinfo in ipairs(Zone_index) do
        ln = getline()
        assert('zone/' .. zinfo[1] ==  ln )
    end

    kz_openfile('world/world_files')

    for iz, zinfo in ipairs(Zone_index) do
        ln = getline()
        assert('world/' .. zinfo[2] ==  ln )
    end

end
]]

function read_assets()
    -- Read an obj or a mob entry
    local oid, oinfo, oxtras
    oinfo = ''; oxtras = ''

    local ln, ch, onr, oname, assets
    -- fixed parts
    oid = getline()
    onr = entry_id_to_num(oid)

    oname = getzstring()
    if not oname then return nil end

    assert(strok(oname), 'empty assets name')

    oinfo = oname .. EOSTR .. getzstring() .. EOSTR
            .. getzstring() .. EOSTR .. getzstring() .. EOSTR
    oinfo = oinfo .. getline() ..  '\n' .. getline() .. '\n'  .. getline() .. '\n'

    -- variable parts
    while true do
        ln = lookahead()
        ch = firstchar(ln)
        if ch == 'E' then
            oxtras = oxtras .. read_xtra(ln)
        elseif ch == 'A' or ch == 'B' then
            oxtras = oxtras .. getline() .. '\n' .. getline() .. '\n'
        elseif ch == COMCHAR then
            oxtras = oxtras .. getline() .. '\n'
        else
            break
        end
    end
     -- Object_index/Mobile_index assets entry struct:
    assets = { onr, oid, oinfo, oname, oxtras, onr, 0, 0 }
    return assets
end

function load_assets_index(assets_index, fn)
    -- Read obj or mob database file

    kz_openfile(fn)

    while true do
        local ln, assets, nr
        ln = lookahead()
        if EOFline(ln) then break end

        if not blankline(ln) then
            assets = read_assets()
            if not assets then break end
            nr = entry_idnum(assets)
            assets_index[nr] = assets
        end
    end
end

function renum_assets_index(assets_index, fn)

    kz_openfile(fn, true)

    for nr, assets in ipairs_sorted(assets_index, G_renum and renum_cmp or nil) do
        -- { onr, oid, oinfo, oname, oxtras, onr, 0, 0 }

        renum_writeln( '#' .. tostring(renum_assets(nr)))
        renum_write(assets[3])
        renum_write(assets[5])
    end
    renum_writeln(EOFENDING)
end

function renum_cmp ( a, b )
    return a[RENUMINDEX] < b[RENUMINDEX]
end

----------------------------------------------------

function read_room()
    -- Read a room in world
    local rid, rnr, rname, rinfo, rexits, rxtras, rooom
    rexits = {} ; rxtras = ''

    -- fixed parts
    rid = getline()
    rnr = entry_id_to_num(rid)
    if EOFline(lookahead()) then return nil end
    rname = getzstring()
    assert (strok(rname), 'empty room name')
    rinfo = rname .. EOSTR .. getzstring() .. EOSTR .. getline() .. '\n'

    -- variable parts
    while true do
        local ln, ch, ex_info
        ln = lookahead()
        ch = firstchar(ln)
        if ch == 'D' then
            assert(ln:match('^D%d'), 'invalid exit direction')
            ex_info = getline() .. '\n' .. getzstring() .. EOSTR .. getzstring() .. EOSTR
            flag =  getline() .. '\n'
            table.insert( rexits, { ex_info,  flag} )
            table.insert( Exit_index, rid .. ' ' .. zone_number() .. ' '.. flag )
        elseif ch == 'E' then
            rxtras = rxtras .. read_xtra()
        elseif ch == COMCHAR then
            rxtras = xtras .. getline() .. '\n'
        else
            break
        end
    end

    ln = getline()
    if trim(ln) ~= 'S' then return nil end

    if rexits.len == 0 then
        rexits = nil
    end
    if rxtras.len == 0 then
        rextras = nil
    end

    -- Room_index room entry struct:  rexits is table
    room = { rnr, rid, rinfo, rexits, rxtras, rnr, 0, 0  }
    return room
end

function read_xtra()
    -- read an extra info
    return getline() .. '\n' .. getzstring() .. EOSTR .. getzstring() .. EOSTR
end

function load_world(wf)
    -- Read a world file

    kz_openfile(wf)

    while true do
        local ln, room, rnr

        ln = lookahead()
        if EOFline(ln) then break end
        if not blankline(ln) then
            room = read_room()
            if not room then break end

            rnr = entry_idnum(room)
            Room_index[rnr] = room
        end
    end
end

function check_world(wf)

    local rmin, rmax = INDEX_OOB, -1

    kz_openfile(wf, true)

    while true do
        local ln, room, rnr, room_id

        ln = lookahead()
        if EOFline(ln) then break end
        if not blankline(ln) then
            room = read_room()
            if not room then break end
            --  { rnr, rid, rinfo, rexits, rxtras, rnr, 0, 0  }
            rnr = entry_idnum(room)
            rmin = math.min(rmin, rnr)
            rmax = math.max(rmax, rnr)

            assert(not out_of_zone(rnr), 'out of zone room number')

            room_id = entry_id(room)
            renum_writeln( '#' .. tostring(renum_room(rnr)) )
            renum_write(room[3])

            for ix, ex in ipairs(room[4]) do
                renum_write (ex[1])
                local flag = ex[2]
                local ex1, key, to_room = flag:match('(%d+)%s+(%-?%d+)%s+(%-?%d+)')
                print_check_entry(key, 'KEY', room_id)
                print_check_entry(to_room, 'EXIT', room_id, -1)  -- dont check oo zone exit
                flag = string.format('%s %d %d', ex1, renum_obj(key), renum_room(to_room))

                renum_writeln (flag)
            end
            renum_write(room[5])
            renum_writeln('S')
        end
    end
    G_room_min = rmin
    G_room_max = rmax
    renum_writeln(EOFENDING)
end

--------------------------------------------------------

function renum_index(index, domain)

    local iz
    for ix, ent in ipairs_sorted(index) do
        assert(ix == ent[1], 'index not in order')
        iz = find_zone(ix)

        if iz and iz > 0 then
            ent[RENUMINDEX] = ix + renum_zone_offset(iz)
        else
            print ('RENUM: NO ZONE ' .. domain .. '   #' .. ix
                .. (domain == 'ROOM' and '' or (' ' .. assets_name(ent))))
        end
    end
end

function renum_room(num)

    local nr, room
    nr = tonumber(num)
    if nr <= 0 then return nr end

    -- if zone_number() > 0 and out_of_zone(nr) then
    --    print ('ROOM OOZ', num, zone_name(), zone_base())
    -- end

    room = Room_index[nr]
    if not room then return nr end

    Room_index[nr][ENTGPVAR] =  Room_index[nr][ENTGPVAR] + 1

    return G_renum and Room_index[nr][RENUMINDEX] or nr
end

function renum_obj(num)

    local nr, obj, name, buf
    nr = tonumber(num)
    if nr <= 0 then return nr end

    obj = Object_index[nr]
    if not obj then return nr end

    if zone_number() > 0 and out_of_zone(nr)  then
        name = assets_name(obj) or "Missing??"
        buf = string.format('OOZ       OBJ: #%-6d "%s"  @ %s (%d) ' , num, name, zone_name(), zone_base())
        print(buf)
    end

    obj[REFCNT] = obj[REFCNT] + 1

    return G_renum and obj[RENUMINDEX] or nr
end

function renum_mob(num)

    local nr, mob
    nr = tonumber(num)
    if nr <= 0 then return nr end

    mob = Mobile_index[nr]
    if not mob then return nr end

    if zone_number() > 0 and out_of_zone(nr)  then
        name = assets_name(mob) or "Missing??"
        buf = string.format('OOZ       MOB: #%-6d "%s"  @ %s (%d) ' , num, name, zone_name(), zone_base())
        print(buf)
    end

    mob[REFCNT] = mob[REFCNT] + 1

    return G_renum and mob[RENUMINDEX] or nr
end

--------------------------------------------------------

function check_zone(zf)
    -- Read zone file

    local zname, ztop, h1, hrest, zinfo, zheader, zdata, ln, ch
    zdata = ''

    kz_openfile(zf, true)

    zname = getzstring()
    ln =  getline()
    h1, hrest = ln:match('(%d+)%s+(.*)$')
    ztop = tonumber(h1)
    assert( ztop == zone_top(), 'zone top mismatch')

    assert( ztop > zone_base(), 'zone top below zone base ')

    if not G_renum then
        assert( zone_base() > G_last_ztop, 'zone base below last zone top')
    end
    G_last_ztop = ztop
    assert ( zname == zone_name(), 'zone name mismatch 2')

    zheader = zname .. EOSTR
        .. tostring(ztop + renum_zone_offset(zone_number())) .. ' ' .. hrest

    while true do
        local ln, ch, qln
        local mob, obj, room, bag = -1,-1, -1, -1

        ln = getline()
        ch = firstchar(ln)

        if blankline(ln) then
            goto continue
        end

        if ch == COMCHAR  then
            zdata = zdata .. ln .. '\n'
            goto continue
        end

        local a1, a2, a3, a4, rest
        if ch == 'G' then    -- 3 args
            a1, a2, a3, rest = ln:match('^G%s+(%d+)%s+(%d+)%s+(%d+)(.*)')
            a4 = ''
        elseif string.find('MEOPRD', ch) then
            a1, a2, a3, a4, rest = ln:match('^%u%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)(.*)')
        end

        if ch == 'S' then
            break
        elseif ch == 'M' then
            mob, room = a2, a4
            a2 = renum_mob(a2)
            a4 = renum_room(a4)
        elseif ch == 'G' or ch == 'E' then
            obj = a2
            a2 = renum_obj(a2)
        elseif ch == 'O' then
            obj, room = a2, a4
            a2 = renum_obj(a2)
            a4 = renum_room(a4)
        elseif ch == 'P' then
            obj, bag = a2, a4
            a2 = renum_obj(a2)
            a4 = renum_obj(a4)
        elseif ch == 'R' then
            room, obj = a2, a4
            a2 = renum_room(a2)
            a4 = renum_obj(a4)
        elseif ch == 'D' then
            room = a2
            a2 = renum_room(a2)
        else
            break
        end

        qln = quote(ln)
        print_check_entry(room, 'ROOM', qln ) -- , zname)   -- check room of zone
        print_check_entry(mob, 'MOB', qln ) --,  zname)
        print_check_entry(obj, 'OBJ', qln ) -- , zname)
        print_check_entry(bag, 'BAG', qln ) -- , zname)

        ln = table.concat( { ch, a1, a2, a3}, ' ')
        if ch ~= 'G' then    -- 3 args
            ln = ln .. ' ' .. a4
        end
        ln = ln .. rest

        zdata = zdata .. ln .. '\n'
    ::continue::
    end

    renum_writeln(zheader)
    renum_write(zdata)
    renum_writeln('S\n$~')

end

--------------------------------------------------------


function check_exits(zone_exit)
    -- zone_exit : show zone exits or not
    zone_exit = false

    for ix, flag in ipairs(Exit_index) do
        local room, zone, ex1, key, to_room
            = flag:match('(#%d+)%s+(%d+)%s+(%d+)%s+(%-?%d+)%s+(%-?%d+)')

        print_check_entry(key, 'KEY', room, zone)

        print_check_entry(to_room, 'EXIT', room, zone_exit and zone or -1 )
    end
end

function check_unreferenced()

    for ix, obj in ipairs_sorted(Object_index) do
        if obj[REFCNT] == 0 then
            print('UNREF: OBJ', entry_id(obj), assets_name(obj))
        end
    end

    for ix, mob in ipairs_sorted(Mobile_index) do
        if mob[REFCNT] == 0 then
            print('UNREF: MOB', entry_id(mob), assets_name(mob))
        end
    end

    for ix, room in ipairs_sorted(Room_index) do
        if room[REFCNT] == 0 then
            print('UNREF: ROOM', entry_id(room))
        end
    end
end

--------------------------------------------------------

function check_shops(fn)
    -- Read shop file

    kz_openfile(fn, true)

    while true do
        local ln, shid, shnr, shinfo
        shinfo = {}

        ln = lookahead()
        if EOFline(ln) then break end

        if not blankline(ln) then

            shid = getline()
            shnr = entry_id_to_num(shid)

            for ix = 1, (12+7+9) do
                ln = getline()
                table.insert(shinfo, ln )
            end
            if not ln then break end

            local room_id  = '#' .. shinfo[24]
            zone = 'Shop ' .. shid

            print_check_entry(shinfo[24], 'ROOM', room_id, zone)
            shinfo[24] = tostring(renum_room(shinfo[24]))

            print_check_entry(shinfo[22], 'MOB', room_id, zone)
            shinfo[22] = tostring(renum_mob(shinfo[22]))

            for ix = 1, 5 do
                print_check_entry(shinfo[ix], 'OBJ', room_id, zone)
                shinfo[ix] = tostring(renum_obj(shinfo[ix]))
            end

            renum_writeln(shid)
            renum_writeln(table.concat(shinfo, '\n'))

        end
    end

    renum_writeln('$~')
end

function check_questmob(fn)
    -- Read quest mob file

    kz_openfile(fn, true)

    while true do

        local ln, qmobnr, qlev, qname, qnr, mob, mlev
        ln = getline()
        if EOFline(ln) then break end

        qmobnr, qlev, qname = ln:match( '(%d+) (%d+)%s+(%S.-)%s*$' )
        -- print (qmob, qlev, qname)
        qnr = tonumber(qmobnr)

        print_check_entry(qnr, 'MOB', qname, 'Quest mob', -1)

        mob = Mobile_index[qnr]
        mlev = mob[3]:match('\n%u (%d%d?) ')
        if mlev ~= qlev then
            print( qmobnr, qname, assets_name(mob), qlev, mlev )
        end

        qnr = renum_mob(qnr)

        ln = string.format('%d %d %s', qnr, qlev, qname )
        renum_writeln(ln)
    end

    renum_writeln(EOFCHAR)

end

--------------------------------------------------------
--------------------------------------------------------

function check_entry_num(ent_num, domain, zone_num)

    local inr, dom, mis, ooz, chk
    inr = tonumber(ent_num)
    dom = domain:upper()

    if inr < 0 or (dom == 'KEY' and inr == 0) then return nil end

    mis = (((dom == 'OBJ' or dom == 'KEY' or dom == 'BAG' ) and not Object_index[inr])
        or ((dom == 'ROOM' or dom == 'EXIT') and not Room_index[inr])
        or (dom == 'MOB' and not Mobile_index[inr]))

    if zone_num >= 0 then
        ooz = out_of_zone(inr, zone_num)
    end

    if mis and ooz then
        chk = 'ZOO+MISS'
    elseif mis then
        chk = 'MISS'
    elseif ooz then
        chk = 'ZOO'
    else
        chk = nil
    end
    return chk, mis, ooz
end

function print_check_entry(ent_num, domain, misc, zone)

    local znum = tonumber(zone)
    if not zone then
        znum = zone_number()
        zone = zone_name()
    elseif znum and znum >= 0 then
        zone = zone_name(znum)
    else
        znum = -1
    end

    local  err, buf
    err  = check_entry_num(ent_num, domain, znum)
    if not err then return end

    buf = string.format( "%-8s %4s: #%-6s %s", err, domain, ent_num, misc )

    if strok(zone) then
        buf = buf .. ' @ ' .. zone
    end
    print (buf)
end

--------------------------------------------------------
