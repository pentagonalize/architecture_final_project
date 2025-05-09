#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define INPUT_NODES 784  // 28*28 pixels
#define HIDDEN_NODES 256 // Number of hidden nodes
#define OUTPUT_NODES 10  // 10 digits (0-9)

// Default values that can be overridden by command line arguments
#define DEFAULT_TRAINING_IMAGES 5
#define DEFAULT_TEST_IMAGES 1
#define DEFAULT_EPOCHS 1

// Global variables for command line arguments
int NUM_TRAINING_IMAGES = DEFAULT_TRAINING_IMAGES;
int NUM_TEST_IMAGES = DEFAULT_TEST_IMAGES;
int NUMBER_OF_EPOCHS = DEFAULT_EPOCHS;

double *training_images;  // Will be dynamically allocated
double *training_labels;  // Will be dynamically allocated
double *test_images;      // Will be dynamically allocated
double *test_labels;      // Will be dynamically allocated

// Initialize weights and biases
double weight1[INPUT_NODES][HIDDEN_NODES];
double weight2[HIDDEN_NODES][OUTPUT_NODES];
double bias1[HIDDEN_NODES];
double bias2[OUTPUT_NODES];

int correct_predictions;
int forward_prob_output;

void load_mnist()
{
    // Change the current working directory to the location of the executable
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *last_slash = strrchr(cwd, '/');
        if (last_slash != NULL) {
            *last_slash = '\0'; // Terminate the string after the last slash
            if (chdir(cwd) != 0) {
                perror("chdir failed");
                exit(1);
            }
        } else {
            printf("Error: Could not determine executable directory.\n");
            exit(1);
        }
    } else {
        perror("getcwd failed");
        exit(1);
    }

    // Define the relative path to the data directory
    char data_dir[] = "mnist_images/";
    char training_images_path[256];
    char training_labels_path[256];
    char test_images_path[256];
    char test_labels_path[256];

    snprintf(training_images_path, sizeof(training_images_path), "%smnist_train_images.bin", data_dir);
    snprintf(training_labels_path, sizeof(training_labels_path), "%smnist_train_labels.bin", data_dir);
    snprintf(test_images_path, sizeof(test_images_path), "%smnist_test_images.bin", data_dir);
    snprintf(test_labels_path, sizeof(test_labels_path), "%smnist_test_labels.bin", data_dir);


    // Allocate memory for training and test data
    training_images = (double*)malloc(NUM_TRAINING_IMAGES * INPUT_NODES * sizeof(double));
    training_labels = (double*)malloc(NUM_TRAINING_IMAGES * OUTPUT_NODES * sizeof(double));
    test_images = (double*)malloc(NUM_TEST_IMAGES * INPUT_NODES * sizeof(double));
    test_labels = (double*)malloc(NUM_TEST_IMAGES * OUTPUT_NODES * sizeof(double));
    
    if (!training_images || !training_labels || !test_images || !test_labels) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // Open the training images file
    FILE *training_images_file = fopen("mnist_train_images.bin", "rb");
    if (training_images_file == NULL)
    {
        printf("Error opening training images file\n");
        exit(1);
    }

    // Open the training labels file
    FILE *training_labels_file = fopen("mnist_train_labels.bin", "rb");
    if (training_labels_file == NULL)
    {
        printf("Error opening training labels file\n");
        exit(1);
    }

    // Open the test images file
    FILE *test_images_file = fopen("mnist_test_images.bin", "rb");
    if (test_images_file == NULL)
    {
        printf("Error opening test images file\n");
        exit(1);
    }

    // Open the test labels file
    FILE *test_labels_file = fopen("mnist_test_labels.bin", "rb");
    if (test_labels_file == NULL)
    {
        printf("Error opening test labels file\n");
        exit(1);
    }

    // Read the training images
    for (int i = 0; i < NUM_TRAINING_IMAGES; i++)
    {
        for (int j = 0; j < INPUT_NODES; j++)
        {
            unsigned char pixel;
            fread(&pixel, sizeof(unsigned char), 1, training_images_file);
            training_images[i * INPUT_NODES + j] = (double)pixel / 255.0;
        }
    }

    // Read the training labels
    for (int i = 0; i < NUM_TRAINING_IMAGES; i++)
    {
        unsigned char label;
        fread(&label, sizeof(unsigned char), 1, training_labels_file);
        for (int j = 0; j < OUTPUT_NODES; j++)
        {
            if (j == label)
            {
                training_labels[i * OUTPUT_NODES + j] = 1;
            }
            else
            {
                training_labels[i * OUTPUT_NODES + j] = 0;
            }
        }
    }

    // Read the test images
    for (int i = 0; i < NUM_TEST_IMAGES; i++)
    {
        for (int j = 0; j < INPUT_NODES; j++)
        {
            unsigned char pixel;
            fread(&pixel, sizeof(unsigned char), 1, test_images_file);
            test_images[i * INPUT_NODES + j] = (double)pixel / 255.0;
        }
    }

    // Read the test labels
    for (int i = 0; i < NUM_TEST_IMAGES; i++)
    {
        unsigned char label;
        fread(&label, sizeof(unsigned char), 1, test_labels_file);
        for (int j = 0; j < OUTPUT_NODES; j++)
        {
            if (j == label)
            {
                test_labels[i * OUTPUT_NODES + j] = 1;
            }
            else
            {
                test_labels[i * OUTPUT_NODES + j] = 0;
            }
        }
    }

    // Close the files
    fclose(training_images_file);
    fclose(training_labels_file);
    fclose(test_images_file);
    fclose(test_labels_file);
}

double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

int max_index(double arr[], int size) {
    int max_i = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] > arr[max_i]) {
            max_i = i;
        }
    }
    return max_i;
}

void train(double *input, double *output, double weight1[INPUT_NODES][HIDDEN_NODES], double weight2[HIDDEN_NODES][OUTPUT_NODES], double bias1[HIDDEN_NODES], double bias2[OUTPUT_NODES], int correct_label)
{
    double hidden[HIDDEN_NODES];
    double hidden_raw[HIDDEN_NODES]; // Raw values before activation
    double output_layer[OUTPUT_NODES];

    // Feedforward
    for (int i = 0; i < HIDDEN_NODES; i++)
    {
        double sum = 0;
        for (int j = 0; j < INPUT_NODES; j++)
        {
            sum += input[j] * weight1[j][i];
        }
        sum += bias1[i];
        hidden_raw[i] = sum;
        hidden[i] = sigmoid(sum);
    }
    for (int i = 0; i < OUTPUT_NODES; i++)
    {
        double sum = 0;
        for (int j = 0; j < HIDDEN_NODES; j++)
        {
            sum += hidden[j] * weight2[j][i];
        }
        sum += bias2[i];
        output_layer[i] = sigmoid(sum); // Always use sigmoid for output layer
    }

    int index = max_index(output_layer, OUTPUT_NODES);

    if (index == correct_label) {
        forward_prob_output++;
    }

    // Backpropagation
    double error[OUTPUT_NODES];
    for (int i = 0; i < OUTPUT_NODES; i++)
    {
        error[i] = output[i] - output_layer[i];
    }
    
    double delta2[HIDDEN_NODES];
    for (int i = 0; i < HIDDEN_NODES; i++)
    {
        delta2[i] = 0;
        for (int j = 0; j < OUTPUT_NODES; j++)
        {
            delta2[i] += error[j] * output_layer[j] * (1 - output_layer[j]) * weight2[i][j];
        }
        delta2[i] *= hidden[i] * (1 - hidden[i]);
    }
    
    double delta1[INPUT_NODES];
    for (int i = 0; i < INPUT_NODES; i++)
    {
        delta1[i] = 0;
        for (int j = 0; j < HIDDEN_NODES; j++)
        {
            delta1[i] += delta2[j] * weight1[i][j];
        }
    }

    // Update weights and biases
    double learning_rate = 0.1;
    for (int i = 0; i < INPUT_NODES; i++)
    {
        for (int j = 0; j < HIDDEN_NODES; j++)
        {
            weight1[i][j] += learning_rate * delta1[i] * input[j];
        }
    }
    for (int i = 0; i < HIDDEN_NODES; i++)
    {
        bias1[i] += learning_rate * delta2[i];
        for (int j = 0; j < OUTPUT_NODES; j++)
        {
            weight2[i][j] += learning_rate * error[j] * output_layer[j] * (1 - output_layer[j]) * hidden[i];
        }
    }
    for (int i = 0; i < OUTPUT_NODES; i++)
    {
        bias2[i] += learning_rate * error[i] * output_layer[i] * (1 - output_layer[i]);
    }
}

void test(double *input, double weight1[INPUT_NODES][HIDDEN_NODES], double weight2[HIDDEN_NODES][OUTPUT_NODES], double bias1[HIDDEN_NODES], double bias2[OUTPUT_NODES], int correct_label)
{
    double hidden[HIDDEN_NODES];
    double output_layer[OUTPUT_NODES];

    // Feedforward
    for (int i = 0; i < HIDDEN_NODES; i++)
    {
        double sum = 0;
        for (int j = 0; j < INPUT_NODES; j++)
        {
            sum += input[j] * weight1[j][i];
        }
        sum += bias1[i];
        hidden[i] = sigmoid(sum);
    }
    for (int i = 0; i < OUTPUT_NODES; i++)
    {
        double sum = 0;
        for (int j = 0; j < HIDDEN_NODES; j++)
        {
            sum += hidden[j] * weight2[j][i];
        }
        sum += bias2[i];
        output_layer[i] = sigmoid(sum); // Always use sigmoid for output layer
    }
    int index = max_index(output_layer, OUTPUT_NODES);

    printf("Prediction: %d\n", index);
    if (index == correct_label) {
        correct_predictions++;
    }
}

void save_weights_biases(char* file_name) {
    FILE* file = fopen(file_name, "wb");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(weight1, sizeof(double), HIDDEN_NODES * INPUT_NODES, file);
    fwrite(weight2, sizeof(double), HIDDEN_NODES * OUTPUT_NODES, file);
    fwrite(bias1, sizeof(double), HIDDEN_NODES, file);
    fwrite(bias2, sizeof(double), OUTPUT_NODES, file);
    fclose(file);
}

void load_weights_biases(char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    fread(weight1, sizeof(double), HIDDEN_NODES * INPUT_NODES, file);
    fread(weight2, sizeof(double), HIDDEN_NODES * OUTPUT_NODES, file);
    fread(bias1, sizeof(double), HIDDEN_NODES, file);
    fread(bias2, sizeof(double), OUTPUT_NODES, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--train") == 0 && i + 1 < argc) {
            NUM_TRAINING_IMAGES = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--test") == 0 && i + 1 < argc) {
            NUM_TEST_IMAGES = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc) {
            NUMBER_OF_EPOCHS = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --train N    Set number of training examples (default: %d)\n", DEFAULT_TRAINING_IMAGES);
            printf("  --test N     Set number of test examples (default: %d)\n", DEFAULT_TEST_IMAGES);
            printf("  --epochs N   Set number of training epochs (default: %d)\n", DEFAULT_EPOCHS);
            printf("  --help       Display this help message\n");
            return 0;
        }
    }

    printf("Neural Network Configuration:\n");
    printf("  Hidden Nodes: %d\n", HIDDEN_NODES);
    printf("  Activation Function: sigmoid\n");
    printf("  Training Examples: %d\n", NUM_TRAINING_IMAGES);
    printf("  Test Examples: %d\n", NUM_TEST_IMAGES);
    printf("  Epochs: %d\n", NUMBER_OF_EPOCHS);
    
    // Initialize weights and biases
    double weight1[INPUT_NODES][HIDDEN_NODES];
    double weight2[HIDDEN_NODES][OUTPUT_NODES];
    double bias1[HIDDEN_NODES];
    double bias2[OUTPUT_NODES];

    for (int i = 0; i < INPUT_NODES; i++)
    {
        for (int j = 0; j < HIDDEN_NODES; j++)
        {
            weight1[i][j] = (double)rand() / RAND_MAX;
        }
    }
    for (int i = 0; i < HIDDEN_NODES; i++)
    {
        bias1[i] = (double)rand() / RAND_MAX;
        for (int j = 0; j < OUTPUT_NODES; j++)
        {
            weight2[i][j] = (double)rand() / RAND_MAX;
        }
    }
    for (int i = 0; i < OUTPUT_NODES; i++)
    {
        bias2[i] = (double)rand() / RAND_MAX;
    }

    // Load MNIST dataset
    load_mnist();

    // Train the network
    for(int epoch=0;epoch<NUMBER_OF_EPOCHS;epoch++)
    {
        forward_prob_output = 0;
        for (int i = 0; i < NUM_TRAINING_IMAGES; i++)
        {
            int correct_label = max_index(&training_labels[i * OUTPUT_NODES], OUTPUT_NODES);
            train(&training_images[i * INPUT_NODES], &training_labels[i * OUTPUT_NODES], weight1, weight2, bias1, bias2, correct_label);
        }
        printf("Epoch %d : Training Accuracy: %f\n",epoch , (double) forward_prob_output / NUM_TRAINING_IMAGES);
    }
    save_weights_biases("model_256_sigmoid.bin");

    // Test the network
    correct_predictions = 0;
    for (int i = 0; i < NUM_TEST_IMAGES; i++)
    {
        int correct_label = max_index(&test_labels[i * OUTPUT_NODES], OUTPUT_NODES);
        test(&test_images[i * INPUT_NODES], weight1, weight2, bias1, bias2, correct_label);
    }
    printf("Testing Accuracy: %f\n", (double) correct_predictions / NUM_TEST_IMAGES);

    // Free dynamically allocated memory
    free(training_images);
    free(training_labels);
    free(test_images);
    free(test_labels);

    return 0;
}
