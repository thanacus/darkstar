local MaxAreas =
{
    -- temenos
    {Max = 8, Zones = {37}},
    
    -- apollyon
    {Max = 6, Zones = {38}},
    
    -- dynamis
    {Max = 1, Zones = {39, 40, 41, 42, 134, 135, 185, 186, 187, 188
                                140 }}, -- ghelsba
};

local function onBattlefieldHandlerInitialise(zone)
    for _, battlefield in pairs(MaxAreas) do
        for _, zoneid in pairs(battlefield.Zones) do
            if zone:getID() == zoneid then
                return battlefield.Max;
             end;
        end
    end;
end;



g_Battlefield = {};

g_Battlefield.Status =
{
    OPEN     = 0,
    LOCKED  = 1,
    WON      = 2,
    LOST     = 3,
};

function g_Battlefield.onBattlefieldTick(battlefield, timeinside)
    local tick = battlefield:getTick();
    local killedallmobs = true;
    
    for _, mob in pairs(battlefield:getMobs(true)) do
        if mob:getHP() > 0 then
            killedallmobs = false;
            break;
        end;
    end;
    g_Battlefield.HandleWipe(battlefield);
    g_Battlefield.HandleTimePrompts(battlefield);
end;

function g_Battlefield.HandleTimePrompts(battlefield)
    local tick = battlefield:getTick();
    local status = battlefield:getStatus();
    
    if tick/1000 % 60 then
        for _, player in pairs(battlefield:getPlayers()) do
            player:messageBasic(202, battlefield:getRemainingTime());
        end;
   end;
end;

function g_Battlefield.HandleWipe(battlefield)
    local rekt = true;
    
    local players = battlefield:getPlayers();
    local totalrekt = 0; 
    
    for _, player in pairs(players) do
        if player:getHP() == 0 then
            if player:getStatusEffect(EFFECT_RERAISE) then
                rekt = false;
                break;
            end;
            totalrekt = totalrekt + 1;
        end;
    end;
    
    if #players == totalrekt then
        if rekt then
            battlefield:setStatus(g_Battlefield.Status.LOST);
            battlefield:cleanup(true);
        else
            battlefield:setWipeTime(battlefield:getStartTime() + battlefield:getTimeInside());
        end;
    end;
end;