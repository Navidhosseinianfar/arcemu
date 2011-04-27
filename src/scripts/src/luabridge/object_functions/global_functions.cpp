#include "LUAEngine.h"


LuaGossip::LuaGossip(uint32 id)
{
	m_go_gossip_binding = NULL, m_item_gossip_binding = NULL, m_unit_gossip_binding = NULL, id_ = id;
}

LuaGossip::~LuaGossip() 
{
	PLUA_INSTANCE ref = lua_instance.get();
	if(this->m_go_gossip_binding != NULL)
	{
		li::GossipInterfaceMap::iterator itr = ref->m_go_gossipInterfaceMap.find(id_);
		if(itr != ref->m_go_gossipInterfaceMap.end() )
			ref->m_go_gossipInterfaceMap.erase(itr);
	}
	else if(this->m_unit_gossip_binding != NULL)
	{
		li::GossipInterfaceMap::iterator itr = ref->m_unit_gossipInterfaceMap.find(id_);
		if(itr != ref->m_unit_gossipInterfaceMap.end() )
			ref->m_unit_gossipInterfaceMap.erase(itr);
	}
	else if(this->m_item_gossip_binding != NULL)
	{
		li::GossipInterfaceMap::iterator itr = ref->m_item_gossipInterfaceMap.find(id_);
		if(itr != ref->m_item_gossipInterfaceMap.end( ) )
			ref->m_item_gossipInterfaceMap.erase(itr);
	}
}

void LuaGossip::OnHello(Object* pObject, Player* Plr)
{
	GET_LOCK;
	if(pObject->GetTypeId() == TYPEID_UNIT)
	{
		if(m_unit_gossip_binding == NULL) { RELEASE_LOCK; return; }

		lua_engine::BeginLuaFunctionCall(m_unit_gossip_binding->refs[GOSSIP_EVENT_ON_TALK]);
		push_unit(TO_UNIT(pObject) );
		push_int(GOSSIP_EVENT_ON_TALK);
		push_player(Plr);
		lua_engine::ExecuteLuaFunction(3);
	}
	else if(pObject->GetTypeId() == TYPEID_ITEM)
	{
		if(m_item_gossip_binding == NULL) { RELEASE_LOCK; return; }

		lua_engine::BeginLuaFunctionCall(m_item_gossip_binding->refs[GOSSIP_EVENT_ON_TALK]);
		push_item( TO_ITEM(pObject) );
		push_int(GOSSIP_EVENT_ON_TALK);
		push_player(Plr);
		lua_engine::ExecuteLuaFunction(3);
	}
	else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
	{
		if(m_go_gossip_binding == NULL) { RELEASE_LOCK; return; }

		lua_engine::BeginLuaFunctionCall(m_go_gossip_binding->refs[GOSSIP_EVENT_ON_TALK]);
		push_go(TO_GAMEOBJECT(pObject));
		push_int(GOSSIP_EVENT_ON_TALK);
		push_player(Plr);
		lua_engine::ExecuteLuaFunction(3);
	}
	RELEASE_LOCK
}

void LuaGossip::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char * EnteredCode)
{
	GET_LOCK;
	if(pObject->GetTypeId() == TYPEID_UNIT)
	{
		if(m_unit_gossip_binding == NULL) { RELEASE_LOCK; return; }

		lua_engine::BeginLuaFunctionCall(m_unit_gossip_binding->refs[GOSSIP_EVENT_ON_SELECT_OPTION]);
		push_unit(TO_UNIT(pObject));
		push_int(GOSSIP_EVENT_ON_SELECT_OPTION);
		push_player(Plr);
		push_int(Id);
		push_str(EnteredCode);
		lua_engine::ExecuteLuaFunction(5);
	}
	else if(pObject->GetTypeId() == TYPEID_ITEM)
	{
		if(m_item_gossip_binding == NULL) { RELEASE_LOCK; return; }
		lua_engine::BeginLuaFunctionCall(m_item_gossip_binding->refs[GOSSIP_EVENT_ON_SELECT_OPTION]);
		push_item(TO_ITEM(pObject));
		push_int(GOSSIP_EVENT_ON_SELECT_OPTION);
		push_player(Plr);
		push_int(Id);
		push_str(EnteredCode);
		lua_engine::ExecuteLuaFunction(5);
	}
	else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
	{
		if(m_go_gossip_binding == NULL) { RELEASE_LOCK; return; }
		lua_engine::BeginLuaFunctionCall(m_go_gossip_binding->refs[GOSSIP_EVENT_ON_SELECT_OPTION]);
		push_go(TO_GAMEOBJECT(pObject));
		push_int(GOSSIP_EVENT_ON_SELECT_OPTION);
		push_player(Plr);
		push_int(Id);
		push_str(EnteredCode);
		lua_engine::ExecuteLuaFunction(5);
	}
	RELEASE_LOCK
}

void LuaGossip::Destroy() 
{
	delete this;
}

namespace lua_engine
{
	Arcemu::Gossip::Script * createunitgossipInterface(uint32 id)
	{
		LuaGossip * pLua = NULL;
		PLUA_INSTANCE ref = lua_instance.get();
		//First check if we have registered a binding.
		li::ObjectBindingMap::iterator itr = ref->m_unitGossipBinding.find(id);
		PObjectBinding pBinding = (itr != ref->m_unitGossipBinding.end() ) ? itr->second : NULL;
		//Don't bother creating an interface if we don't have a binding.
		if( pBinding != NULL )
		{
			//Check if we had already created an interface, if so, we just update its binding to point to the newly registered one.
			li::GossipInterfaceMap::iterator itr = ref->m_unit_gossipInterfaceMap.find(id);
			if(itr != ref->m_unit_gossipInterfaceMap.end() )
				pLua = itr->second;
			else
			{
				pLua = new LuaGossip(id);
				//All units w/ the same id will share this interface.
				ref->m_unit_gossipInterfaceMap.insert(make_pair(id,pLua));
			}
			pLua->m_unit_gossip_binding = pBinding;
		}
		return pLua;
	}
	Arcemu::Gossip::Script * createitemgossipInterface(uint32 id)
	{
		LuaGossip * pLua = NULL;
		PLUA_INSTANCE ref = lua_instance.get();
		li::ObjectBindingMap::iterator itr = ref->m_itemGossipBinding.find(id);
		PObjectBinding pBinding = (itr != ref->m_itemGossipBinding.end() ) ? itr->second : NULL;
		if( pBinding != NULL )
		{
			li::GossipInterfaceMap::iterator itr = ref->m_item_gossipInterfaceMap.find(id);

			if(itr != ref->m_item_gossipInterfaceMap.end() )
				pLua = itr->second;
			else
			{
				pLua = new LuaGossip(id);
				ref->m_item_gossipInterfaceMap.insert(make_pair(id,pLua));
			}
			pLua->m_item_gossip_binding = pBinding;
		}
		return pLua;
	}
	Arcemu::Gossip::Script * creategogossipInterface(uint32 id)
	{
		LuaGossip * pLua = NULL;
		PLUA_INSTANCE ref = lua_instance.get();
		li::ObjectBindingMap::iterator itr = ref->m_goGossipBinding.find(id);
		PObjectBinding pBinding = (itr != ref->m_goGossipBinding.end() ) ? itr->second : NULL;
		if( pBinding != NULL )
		{
			li::GossipInterfaceMap::iterator itr = ref->m_go_gossipInterfaceMap.find(id);
			if(itr != ref->m_go_gossipInterfaceMap.end() )
				pLua = itr->second;
			else
			{
				pLua = new LuaGossip(id);
				ref->m_go_gossipInterfaceMap.insert(make_pair(id,pLua));
			}
			pLua->m_go_gossip_binding = pBinding;
		}
		return pLua;
	}
}

int CreateLuaEvent(lua_function fref, int delay, int repeats, variadic_parameter* params)
{
	int ref = LUA_REFNIL;
	PLUA_INSTANCE li_ = lua_instance.get();
	if(delay > 0 && (ptrdiff_t)fref != LUA_REFNIL && params != NULL)
	{
		//embed the function ref and repeats as part of our parameters.
		variadic_node * func_node = new variadic_node;
		variadic_node * repeats_node = new variadic_node;
		repeats_node->type = CUSTOM_TYPE_REPEATS_ARG;
		repeats_node->val.bewl = repeats;
		func_node->type = LUA_TFUNCTION;
		func_node->val.obj_ref = (ptrdiff_t)fref;
		//make the function the new head node.
		func_node->next = repeats_node;
		repeats_node->next = params->head_node;
		params->head_node = func_node;
		//update args count
		params->count +=2;
		TimedEvent * ev = TimedEvent::Allocate(li_->map,new CallBackFunctionP1<variadic_parameter*>(&lua_engine::ExecuteLuaFunction,params),0,delay,repeats);
		ev->eventType  = LUA_EVENTS_END+(ptrdiff_t)fref; //Create custom reference by adding the ref number to the max lua event type to get a unique reference for every function.
		li_->map->event_AddEvent(ev);
		li_->m_globalFRefs.insert(params);
	}
	return ref;
}
void lua_engine::ExecuteLuaFunction(variadic_parameter * params)
{
	if(params != NULL )
	{
		PLUA_INSTANCE li_ = lua_instance.get();
		lua_State * lu = li_->lu;
		//place the function on the stack.
		ReferenceHandler::getReference(lu,params->head_node->val.obj_ref);
		int arg_cnt = params->count - 2;
		if(arg_cnt > 0)
		{
			//retrieve the repeats.
			variadic_node * function_node = params->head_node;
			uint16 repeats = (uint16)params->head_node->next->val.bewl;
			/*	Prepare to push arguments, 1st assign the head node to the actual arguments registered to this function */
			params->head_node = function_node->next->next;
			//subtract the function n repeat node from arg count.
			params->count-= 2;
			//Now we push all args.
			luabridge::tdstack<variadic_parameter*>::push(lu, params);
			//call the function
			if(lua_pcall(lu, arg_cnt, 0,0) )
				report(lu);
			//if it's not an infinite/one time call event.
			if(repeats > 1)
				//decrement repeats and put it back in the params.
				function_node->next->val.bewl = (int)--repeats;
			else if(repeats == 1)
			{
				//reset our function node as the starting node.
					
				variadic_node * repeats_node = function_node->next;
				function_node->next = params->head_node;
				params->head_node = function_node;
				//de-allocate repeats node
				delete repeats_node;
				//remove this function from storage.
				li_->m_globalFRefs.erase(params);
				//since we've put the function node back.
				params->count++;
				//clean up the rest of the args
				cleanup_varparam(params, lu);
			}
		}	
	}
}
void ModifyLuaEventInterval(lua_function ref, int newInterval)
{
	PLUA_INSTANCE li_ = lua_instance.get();
	if(li_ != NULL)
	//Easy interval modification.
		sEventMgr.ModifyEventTime(li_->map,(size_t)ref+LUA_EVENTS_END, newInterval);
}
static void DestroyLuaEvent(lua_function ref)
{
	PLUA_INSTANCE li_ = lua_instance.get();
	if(li_ != NULL)
	{
		//Simply remove the reference, CallFunctionByReference will find the reference has been freed and skip any processing.
		lua_unref( li_->lu,(ptrdiff_t)ref);
		ReferenceHandler::removeReference( li_->lu, (ptrdiff_t)ref );
		for(li::References::iterator itr = li_->m_globalFRefs.begin(); itr != li_->m_globalFRefs.end(); ++itr)
		{
			if( (*itr) != NULL && (*itr)->head_node->type == LUA_TFUNCTION && (*itr)->head_node->val.obj_ref == (ptrdiff_t)ref)
			{
				li_->m_globalFRefs.erase(itr);
				break;
			}
		}
		sEventMgr.RemoveEvents( li_->map ,(size_t)ref+LUA_EVENTS_END);
	}
}

//Used to clean up any pending events when restarting.
void DestroyAllLuaEvents(PLUA_INSTANCE instance)
{
	//Clean up for all events.
	li::References::iterator itr = instance->m_globalFRefs.begin();
	ptrdiff_t ref = LUA_REFNIL;
	GET_LOCK;
	for(; itr != instance->m_globalFRefs.end(); ++itr)
	{
		if( (*itr) != NULL && (*itr)->head_node != NULL && (*itr)->head_node->type == LUA_TFUNCTION)
		{
			ref = (*itr)->head_node->val.obj_ref;
			sEventMgr.RemoveEvents(World::getSingletonPtr(),ref+LUA_EVENTS_END);
			cleanup_varparam( (*itr), instance->lu );
		}
	}
	instance->m_globalFRefs.clear();
}
static void GetRegistryTable(const char * name, lua_stack stack)
{
	if(name == NULL)
		lua_pushvalue( (lua_thread)stack, LUA_REGISTRYINDEX);
	else
	{
		lua_getfield( (lua_thread)stack, LUA_REGISTRYINDEX, name);
		if(lua_type( (lua_thread)stack, -1) != LUA_TTABLE)
		{
			lua_pop( (lua_thread)stack, 1);
			lua_pushnil( (lua_thread) stack);
		}
	}
}

//Lua object casting methods
static void lua_toplayer(Object * obj, lua_stack s)
{
	if(obj != NULL && obj->IsPlayer() )
		luabridge::tdstack<Player*>::reassignto( (lua_thread)s, 1);
	else
		luabridge::tdstack<void>::push( (lua_thread)s);
}
static void lua_togo(Object * obj, lua_stack s)
{
	if(obj != NULL && obj->IsGameObject() )
		luabridge::tdstack<GameObject*>::reassignto( (lua_thread)s, 1);
	else
		luabridge::tdstack<void>::push( (lua_thread)s);
}
static void lua_toitem(Object * obj, lua_stack s)
{
	if(obj != NULL && obj->IsItem() )
		luabridge::tdstack<Item*>::reassignto( (lua_thread)s, 1);
	else
		luabridge::tdstack<void>::push( (lua_thread)s);
}
static void lua_tounit(Object * obj, lua_stack s)
{
	if(obj != NULL && obj->IsUnit() )
		luabridge::tdstack<Unit*>::reassignto( (lua_thread)s, 1);
	else
		luabridge::tdstack<void>::push( (lua_thread)s);
}
static void lua_tocreature(Object * obj, lua_stack s)
{
	if(obj != NULL && obj->IsCreature() )
		luabridge::tdstack<Creature*>::reassignto( (lua_thread)s, 1);
	else
		luabridge::tdstack<void>::push( (lua_thread)s);
}

static bool include(const char * filename, bool once)
{
	PLUA_INSTANCE ref = lua_instance.get();
	if( !ref->scripts_.isLoaded(filename) )
	{
		if(lua_engine::loadScript(filename) )
			ref->scripts_.add(filename);
		else
			return false;
	}
	else if(once)
		luaL_error(ref->lu, "%s has already been loaded!", filename);
	return true;
}

#include "GlobalFunctions.h"

namespace lua_engine
{
	void bindGlobalMethods(luabridge::module & m)
	{
		m	.function(&CreateLuaEvent, "CreateLuaEvent", "createEvent", "createevent", "createvent", NULL)
			.function( &ModifyLuaEventInterval, "ModifyLuaEventInterval", "ModifyLuaEventTimer", "modifyeventtime", "modifyEventTime", NULL)
			.function( &DestroyLuaEvent, "DestroyLuaEvent", "destroyevent", "destroyEvent", NULL)
			//Used to retrieve object method tables.
			.function( &GetRegistryTable, "getregistry", "GetRegistry", "getRegistry", NULL)
			//Casting global functions
			.function( &lua_toplayer, "TO_PLAYER", "to_player", "toplayer", "TOPLAYER", NULL)
			.function( &lua_tocreature, "TO_CREATURE", "TOCREATURE", "to_creature", "tocreature", NULL)
			.function( &lua_toitem, "TO_ITEM", "TOITEM", "toitem", "to_item", NULL)
			.function( &lua_tounit, "TO_UNIT", "TOUNIT", "to_unit", "tounit", NULL)
			.function( &lua_togo, "TO_GAMEOBJECT", "TOGAMEOBJECT", "to_gameobject", "togameobject", NULL)
			.function( &GetGameTime, "GetGameTime", "getGameTime", "getgametime", NULL)
			.function( &GetPlayer, "GetPlayer", "getPlayer", "getplayer", NULL)
			.function( &enginename, "GetEngineName", "getEngineName", "getenginename")
			.function( &engineversion, "GetEngineVersion", "getEngineVersion", "getengineversion", "revision", NULL)
			.function( &logcolor, "logcolor", "setConsoleColor", "SetConsoleColor", "setconsolecolor", NULL)
			.function( &SendWorldMessage, "SendWorldMessage", "sendWorldMessage", "sendworldmessage", NULL)
			.function( &ReloadTable, "reloadTable", "ReloadTable", "reloadtable", NULL)
			.function( &ReloadLuaEngine, "reloadLuaEngine", "ReloadLuaEngine", "reloadEngine", "reloadengine", NULL)
			.function( &GetPlayersInWorld, "getPlayersInWorld", "GetPlayersInWorld",  "getplayersinworld", NULL)
			.function( &Rehash, "Rehash", "rehash", NULL)
			.function( &GetArcemuRevision, "GetArcemuRevision", "getrevision", NULL)
			.function( &GetPlayersInMap, "GetPlayersInMap", "getPlayersInMap", "getplayersinmap", NULL)
			.function( &GetPlayersInZone, "GetPlayersInZone", "getplayersinzone", "getPlayersInZone", NULL)
			.function( &luabit_and, "bit_and", "bitand", "bAnd", NULL)
			.function( &luabit_or, "bit_or", "bitor", "bOr", NULL)
			.function( &luabit_xor, "bit_xor", "bitxor", "bXor", "xor", NULL)
			.function( &luabit_not, "bit_not", "bitnot", "bNot",NULL)
			.function( &bit_shiftleft, "bit_shiftleft", "shl", NULL)
			.function( &bit_shiftright, "bit_shiftright", "shr", NULL)
			.function( &GetPlatform, "GetPlatform", "getplatform", "getPlatform", NULL)
			.function( &SendPacketToZone, "SendPacketToZone", "sendpackettozone", "sendpacketozone", NULL)
			.function( &SendPacketToInstance, "sendPacketToInstance", "SendPacketToInstance", NULL)
			.function( &SendPacketToWorld, "SendPacketToWorld", "sendPacketToWorld", "sendpacketoworld", NULL)
			.function( &SendPacketToChannel, "sendpacketochannel", "sendpackettochannel", "SendPacketToChannel", "sendPacketToChannel", NULL)
			.function( &GetInstanceCreature, "GetInstanceCreature", "getInstanceCreature", "getinstancecreature", NULL)
			.function( &GetInstancePlayerCount, "GetInstancePlayerCount", "getInstancePlayerCount", "getinstanceplayercount", NULL)
			.function( &GetPlayersInInstance, "GetPlayersInInstance", "getPlayersInInstance", "getplayersininstance", NULL)
			.function( &include, "include", "Include", "INCLUDE", NULL)
			.function( &WorldDBQuery, "WorldDBQuery", "worlddbquery", NULL)
			.function( &CharDBQuery, "CharDBQuery", "chardbquer", NULL);

		m	.class_<DBCStorage<SpellEntry> >("dbcSpell")
			.method( &DBCStorage<SpellEntry>::LookupEntry, "LookupEntry", "lookupentry", NULL)
			.method( &DBCStorage<SpellEntry>::GetNumRows, "GetNumRows", "getNumRows", "getnumrows", NULL)
			.method( &DBCStorage<SpellEntry>::LookupEntryForced, "LookupEntryForced", "lookupentryforced", NULL);
		//now we overwrite our dbcSpell static table with dbcSpell storage object.
		luabridge::tdstack<DBCStorage<SpellEntry>*>::push(m.L, &dbcSpell);
		lua_setglobal(m.L, "dbcSpell");

		//Expose the world database and character database.
		m	.class_<Database>("WoW_Database")
			.method("Query", &Database::QueryNA)
			.method("Execute", &Database::ExecuteNA);
		
		luabridge::tdstack<Database*>::push(m.L, Database_World);
		lua_setglobal(m.L, "WorldDB");
		luabridge::tdstack<Database*>::push(m.L, Database_Character);
		lua_setglobal(m.L, "CharacterDB");
	}
}