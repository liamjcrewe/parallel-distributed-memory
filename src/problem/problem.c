static const double problem[10][10] = {
    {39.305479, 7.185631, 68.904397, 76.204575, 70.287806, 27.154123, 79.348461, 9.583932, 77.147911, 24.931782},
    {28.452752, 5.401047, 75.399205, 34.439732, 28.572773, 22.595287, 58.989926, 43.679843, 27.120158, 8.497783},
    {22.239443, 78.310198, 59.499364, 5.816251, 53.725788, 69.319290, 49.310187, 56.308894, 83.581101, 47.559062},
    {25.151030, 13.353930, 39.499176, 62.656276, 64.028447, 26.114401, 4.732921, 46.209116, 36.620029, 72.824819},
    {66.732947, 80.637109, 67.895511, 19.857469, 44.474061, 75.546254, 5.897575, 20.539351, 4.869968, 49.549504},
    {78.513282, 72.733960, 39.673691, 95.720421, 73.123719, 90.336935, 92.858463, 72.195639, 92.106730, 37.805212},
    {92.193064, 88.833638, 26.959278, 4.581903, 8.043215, 82.316213, 88.586409, 71.773919, 4.255587, 23.657041},
    {3.895257, 67.578855, 97.817586, 20.161453, 53.539653, 40.952796, 93.634476, 14.634176, 56.603488, 34.823261},
    {74.545023, 78.202674, 52.348440, 20.229938, 4.562215, 77.147453, 17.248635, 97.806132, 27.656248, 18.564696},
    {16.842764, 76.339555, 38.895549, 17.497376, 78.398233, 39.104213, 24.508311, 11.186983, 19.629920, 20.058460}
};

/**
 * Fill the given two dimensional array with double values.
 *
 * Does this using a predefined 10x10 array of values, and repeating these
 * values for the specified (problemDimension) number of times. Uses modular
 * arithmetic to allow any problem dimension to be generated (does not have to
 * be a multiple of 10).
 *
 * @param  problem          The array to fill
 * @param  problemDimension The dimension of the array to fill
 */
void fillProblemArray(double ** const problem, const int problemDimension)
{
    for (int row = 0; row < problemDimension; row++) {
        for (int col = 0; col < problemDimension; col++) {
            problem[row][col] = problem[row % 10][col % 10];
        }
    }
}
