# noinspection PyUnresolvedReferences
import pylibstride as stride


if __name__ == '__main__':

    # The simulation file to load
    # TODO: don't forget to change the paths to the data files to use!
    default_file = "py_test/run_default.xml"

    # Create an instance of the MDP
    mdp = stride.MDP()
    mdp.Create(default_file)

    # Note: Internal Simulation stops after days given in config file have passed,
    # even if Simulate_Day is called afterwards
    for i in range(10):
        # TODO: Bandit selects action to vaccinate
        # mdp.Vaccinate(availableVaccines=0, ageGroup=0, vaccineType=0)

        # Run the simulation for a day
        mdp.Simulate_Day()
