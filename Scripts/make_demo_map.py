# Makes the NsDungeons demo map: /NsDungeons/Demo/L_NsDungeonsDemo with a demo actor, a sun, a sky light, and the base game mode.
# Run in the editor: UnrealEditor-Cmd.exe <Project>.uproject -run=pythonscript -script="<absolute path>/make_demo_map.py" -unattended
import unreal

MAP = "/NsDungeons/Demo/L_NsDungeonsDemo"


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if unreal.EditorAssetLibrary.does_asset_exist(MAP):
        levels.load_level(MAP)
    else:
        levels.new_level(MAP)
    existing = actors.get_all_level_actors()
    if not any(isinstance(a, unreal.NsDungeonDemoActor) for a in existing):
        actors.spawn_actor_from_class(unreal.NsDungeonDemoActor, unreal.Vector(0.0, 0.0, 0.0))
    if not any(isinstance(a, unreal.DirectionalLight) for a in existing):
        sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 1000.0), unreal.Rotator(0.0, -50.0, 30.0))
        sun.light_component.set_editor_property("intensity", 3.0)
    if not any(isinstance(a, unreal.SkyLight) for a in existing):
        sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 800.0))
        sky.light_component.set_editor_property("real_time_capture", True)
    if not any(isinstance(a, unreal.SkyAtmosphere) for a in existing):
        actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0))
    # The base game mode: the demo needs no game rules
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", unreal.GameModeBase)
    levels.save_current_level()
    unreal.log_warning("NSDUNGEONS demo map saved: " + MAP)


main()
