#include<linux/module.h>
#include<linux/spi/spi.h>
#include<linux/iio/iio.h>
#include<linux/iio/driver.h>

#define MCP3008_NUM_CHANNELS 2

struct mcp3008_data{
        struct spi_device *spi;
};

static int mcp3008_read_raw(struct iio_dev *indio_dev,struct iio_chan_spec const *chan,int *val,int *val2,long mask){
        struct mcp3008_data *data = iio_priv(indio_dev);
        u8 tx_buf[3];
        u8 rx_buf[3];

        struct spi_transfer t ={
                .tx_buf = tx_buf,
                .rx_buf = rx_buf,
                .len =3,
        };

        if(mask != IIO_CHAN_INFO_RAW)
                return -EINVAL;

        tx_buf[0] = 0x01;
        tx_buf[1] = (0x80 | (chan->channel<<4));
        tx_buf[2] = 0x00;

        int ret = spi_sync_transfer(data->spi,&t,1);

        if(ret)
                return ret;

        *val = ((rx_buf[1]&0x03) << 8)|rx_buf[2];
        return IIO_VAL_INT;
}

static const struct iio_chan_spec mcp3008_channels[] = {
                {
                        .type = IIO_VOLTAGE,
                        .indexed = 1,
                        .channel = 0,
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                },
                {
                        .type = IIO_VOLTAGE,
                        .indexed = 1,
                        .channel = 1,
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                }
};

static const struct iio_info mcp3008_info ={
        .read_raw = mcp3008_read_raw,
};

static int mcp3008_probe(struct spi_device *spi){
        struct iio_dev *indio_dev;
        struct mcp3008_data *data;

        indio_dev = devm_iio_device_alloc(&spi->dev,sizeof(*data));
        if(!indio_dev)
                return -ENOMEM;
        data = iio_priv(indio_dev);
        data->spi = spi;
        indio_dev->dev.parent = &spi->dev;
        indio_dev->name = "mcp3008";
        indio_dev->modes = INDIO_DIRECT_MODE;
        indio_dev->info = &mcp3008_info;
        indio_dev->channels = mcp3008_channels;
        indio_dev->num_channels = MCP3008_NUM_CHANNELS;

        return devm_iio_device_register(&spi->dev,indio_dev);
}

static const struct of_device_id mcp3008_dt_ids[] = {
        {.compatible = "mcp3008"},
        {}
};

MODULE_DEVICE_TABLE(of,mcp3008_dt_ids);

static struct spi_device_id mcp3008_id[] = {
        {"mcp3008",0},
        {}
};

MODULE_DEVICE_TABLE(spi,mcp3008_id);


static struct spi_driver mcp3008_driver = {
        .driver= {
                .name = "mcp3008",
                .of_match_table= mcp3008_dt_ids,
        },
        .probe = mcp3008_probe,
        .id_table = mcp3008_id,
};

module_spi_driver(mcp3008_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vijay <vijaytarunkoppadi125@gmail.com>");
MODULE_DESCRIPTION("spi mcp3008 device driver");

