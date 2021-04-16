# noinspection PyUnresolvedReferences
import pylibstride as stride
import random


if __name__ == '__main__':

    # The simulation file to load
    # TODO: don't forget to change the paths to the data files to use!
    default_file = "py_test/run_default.xml"

    # Create an instance of the MDP
    mdp = stride.MDP()
    mdp.Create(default_file)
    # Get the number of days to run the experiment for (from the configuration file)
    numDays = mdp.GetNumberOfDays()
    # The step size in days between the next action the bandit can take
    step_size = 10

    all_age_groups = list(stride.AgeGroup.__members__.values())
    print(f"All age groups {all_age_groups}")

    def random_age_group():
        return random.choice(all_age_groups)

    # Note: Internal Simulation stops after days given in config file have passed,
    # even if Simulate_Day is called afterwards
    for i in range(numDays // step_size):
        # TODO: Bandit selects action to vaccinate instead of random choice
        age_group = random_age_group()
        print(f"Vaccinating {age_group.name}")
        mdp.Vaccinate(availableVaccines=10000, ageGroup=age_group, vaccineType=0)

        # Run the simulation
        infected = mdp.Simulate(step_size)
        print("Number infected:", infected)

    # Stop the mdp (simulator)
    mdp.End()
