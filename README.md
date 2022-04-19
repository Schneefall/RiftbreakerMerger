# RiftbreakerMerger
Tool to merge modded files for The Riftbreaker.

## How to use

Place the .exe in the main riftbreaker directory. Run the .exe.  
It will attempt to merge modded versions of the files:
- "scripts/research/research_tree.rt"
- "scripts/research/research_tree_prologue.rt"
- "scripts/research/research_tree_survival.rt"
- "scripts/blueprint_tables/weapon_stats.dat"

by recursively adding new and updating existing values, so the load-order still matters.
It is currently not possible to remove values.
The results are packed into "zzz_ResearchMerge.zip".  

## For Mod Authors

Mod authors can provide a .merge file, e.g. "scripts/research/research_tree.rt.merge", containing only the intended changed for increased compatibility. If a .merge file is available the base file in the same archive will be ignored, meaning you can also provide a version of the mod that does not need to be merged.
Some list stuctures (like `ResearchNode`s in `nodes`) require that a key is present (`research_name` for `ResearchNode`, `category` for `ResearchTree`).  
So a minimal file that adds my [Bioscanner Drones](https://www.nexusmods.com/theriftbreaker/mods/169) as reward to the Alien Research node would look like this:
```
Research
{
	categories
	{
		ResearchTree
		{
			category "gui/menu/research/category_base_buildings"
			nodes
			{
				ResearchNode
				{
					research_name "gui/menu/research/name/alien_research"
					research_awards
					{
						ResearchAward
						{
							blueprint "buildings/main/tower_drone_scanner"
							is_visible "1"
						}

					}

				}

			}

		}

	}

}
```
